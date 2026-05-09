# Project Outline: Custom Vitis HLS Video IP

## Project title
**Real-Time Video Preprocessing and Motion Detection IP for PYNQ**

---

## Project team
Replace with your actual names before submission.

- Sora Kakigi
- Thinh 
- Yi Chieh Huang
- Justin Lu

---

## 1. Project overview
This project will design a **custom Vitis HLS IP core** for the PYNQ video pipeline. The **advanced objective** is to perform **real-time motion detection using frame differencing**, but the project will be organized so that the early work is **directly reused** in the final motion-detection design instead of being thrown away.

The idea is to build the project in **two milestones**:

### Milestone 1: Reusable streaming video preprocessing pipeline
Build a real-time AXI4-Stream video IP that performs:
- pixel unpack / input adaptation,
- RGB-to-grayscale conversion,
- optional denoising filter,
- thresholding / visualization,
- overlay or formatted video output.

This milestone creates the reusable front half of the final system.

### Milestone 2: Motion detection extension
Add:
- storage for the previous frame,
- frame differencing,
- motion thresholding,
- optional mask cleanup,
- motion-highlight overlay.

This second milestone reuses the same input, grayscale, filtering, output, and control infrastructure from Milestone 1.

---

## 2. Final project objective
### Intended functionality
The final custom IP will accept a live video stream and produce one of several output modes in real time:

- **Bypass mode**: original video passes through unchanged.
- **Preprocessing demo mode**: grayscale / filtered / thresholded output.
- **Motion mask mode**: binary image showing moving pixels.
- **Motion overlay mode**: original video with moving regions highlighted.

The final target is a **motion detection accelerator using frame differencing**, where the current frame is compared against the previous frame at each pixel location.

---

## 3. IP definition and mathematical operations

## 3.1 Milestone 1 operations: preprocessing pipeline

### 3.1.1 RGB to grayscale conversion
For each input pixel with channels `R`, `G`, and `B`, compute luminance:

\[
Y = 0.299R + 0.587G + 0.114B
\]

In hardware, this will likely be implemented with an integer approximation such as:

\[
Y \approx (77R + 150G + 29B) >> 8
\]

This reduces the problem from three channels to one channel and is especially useful for later motion detection.

### 3.1.2 Denoising filter
A small 3x3 blur will be used to reduce pixel-level noise before thresholding or motion detection:

\[
G(x,y) = \frac{1}{16}
\begin{bmatrix}
1 & 2 & 1 \\
2 & 4 & 2 \\
1 & 2 & 1
\end{bmatrix} * Y(x,y)
\]

This is useful because small brightness noise between consecutive frames can otherwise produce false motion.

### 3.1.3 Thresholding / Binary visualization
For debugging and early validation, a thresholded output can be generated:

\[
B(x,y) =
\begin{cases}
255 & \text{if } G(x,y) > T \\
0 & \text{otherwise}
\end{cases}
\]

This is not the final motion detector, but it helps validate the grayscale, filter, and control logic.

---

## 3.2 Milestone 2 operations: motion detection using frame differencing

Let `I_t(x,y)` be the current grayscale frame and `I_{t-1}(x,y)` be the previous grayscale frame.

### 3.2.1 Frame differencing
\[
D(x,y) = | I_t(x,y) - I_{t-1}(x,y) |
\]

This measures how much the pixel changed from one frame to the next.

### 3.2.2 Motion thresholding
\[
M(x,y) =
\begin{cases}
255 & \text{if } D(x,y) > T_m \\
0 & \text{otherwise}
\end{cases}
\]

where `T_m` is a motion threshold.

### 3.2.3 Optional post-processing cleanup
A small cleanup stage may be added to reduce isolated false positives, for example a 3x3 majority filter or morphological cleanup.

### 3.2.4 Motion overlay
The motion mask can be combined with the original video. One simple overlay method is:

\[
O(x,y) =
\begin{cases}
\text{highlight}(RGB_t(x,y)) & \text{if } M(x,y)=255 \\
RGB_t(x,y) & \text{otherwise}
\end{cases}
\]

For example, moving pixels could be shown in red while static regions retain the original color.

---

## 4. Why these operations are well suited for hardware acceleration
These operations are good candidates for programmable logic because they are:

- repeated for every pixel,
- dominated by simple arithmetic,
- naturally expressed as a streaming pipeline,
- latency-sensitive for real-time video,
- and easy to pipeline to high throughput.

The basic preprocessing stages can be designed for **one-pixel-per-clock** throughput after pipeline fill. The motion-detection stage adds previous-frame access, but the per-pixel arithmetic remains simple.

---

## 5. Python-style pseudocode

### 5.1 Milestone 1: preprocessing pipeline
```python
for each input pixel:
    y = rgb_to_gray(r, g, b)
    y_filt = blur3x3(y)          # optional
    out = 255 if y_filt > T else 0
```

### 5.2 Milestone 2: motion detection pipeline
```python
for each pixel position (x, y):
    curr = grayscale(current_frame[x, y])
    curr_filt = blur3x3(curr)              # optional / if enabled

    prev = previous_frame[x, y]
    diff = abs(curr_filt - prev)

    if diff > motion_threshold:
        motion_mask[x, y] = 255
    else:
        motion_mask[x, y] = 0

    previous_frame[x, y] = curr_filt
```

Optional cleanup:
```python
motion_mask = majority_filter_3x3(motion_mask)
```

---

## 6. IP architecture
The design will use a **modular architecture** so that Milestone 1 modules can be reused in Milestone 2.

### 6.1 Top-level interface strategy
The design will use a **hybrid interface model**:

- **AXI4-Stream** for the live video pixel path
- **AXI4-Lite** for configuration and control
- **AXI memory-mapped master interface** only when Milestone 2 is added, so the IP can read/write the previous frame stored in external memory

This approach keeps the main design compatible with the PYNQ video pipeline while still allowing frame differencing, which needs access to data from an earlier frame.

---

## 6.2 Module breakdown

### Module A: AXI4-Stream input adapter
Receives video pixels and sideband signals from the PYNQ pipeline. It unpacks the AXI stream into an internal pixel representation and forwards synchronization signals needed for output.

### Module B: Color-to-grayscale converter
Converts the incoming RGB/BGR pixel stream to grayscale luminance. This module is required in both Milestone 1 and Milestone 2.

### Module C: Optional denoising filter
Implements a small streaming 3x3 blur using line buffers and a sliding window. This module is built early and reused later before frame differencing to reduce false motion caused by noise.

### Module D: Threshold / visualization stage
Provides a simple thresholded or filtered output for early testing. This helps validate the grayscale and filtering pipeline before the motion-detection logic is added.

### Module E: Output formatter / overlay engine
Formats the processed pixels back into AXI4-Stream video output. In the early milestone it can output grayscale or thresholded video; later it can display a motion mask or color overlay.

### Module F: Previous-frame buffer interface (Milestone 2)
Reads the previous grayscale frame from external memory and writes the current grayscale frame back for use by the next frame. This is the major new block added in the motion-detection phase.

### Module G: Frame differencing engine (Milestone 2)
Computes the absolute difference between the current filtered grayscale pixel and the stored previous-frame pixel:

`diff = abs(curr - prev)`

This module produces the raw motion-strength value.

### Module H: Motion decision and cleanup stage (Milestone 2)
Applies a programmable motion threshold and optionally performs small post-processing cleanup to reduce isolated false detections. It produces the binary motion mask used by the output stage.

### Module I: AXI4-Lite control register block
Provides software-visible registers for:
- enable / disable,
- bypass,
- grayscale enable,
- filter enable,
- threshold values,
- motion threshold,
- output mode,
- debug/status counters.

---

## 7. Build plan by milestone

## Milestone 1: reusable streaming video pipeline
### Goal
Create a complete and demonstrable custom video IP even before motion detection is finished.

### Deliverables
- AXI4-Stream pass-through integrated into the PYNQ pipeline
- grayscale conversion
- 3x3 denoising filter
- threshold / binary output mode
- output formatting and AXI4-Lite control registers

### Why this matters
This milestone proves that the live video path, control path, and streaming architecture all work correctly. None of this work is discarded when motion detection is added.

---

## Milestone 2: motion detection extension
### Goal
Extend the working pipeline to detect moving regions in live video.

### New deliverables
- previous-frame storage and retrieval
- absolute frame differencing
- programmable motion threshold
- motion mask output
- optional motion overlay on the original video

### Reused work from Milestone 1
- stream input/output
- grayscale conversion
- denoising filter
- output formatting
- AXI4-Lite configuration

---

## 8. Host computer and PYNQ integration
The host processor (PS) will configure the IP through **AXI4-Lite** registers. The live video path will remain in the programmable logic.

### Planned host/IP interaction
1. The PYNQ overlay configures HDMI input and HDMI output.
2. The host sets control registers such as mode, filter enable, and threshold values.
3. The video stream is sent through the custom HLS IP in the PL.
4. In Milestone 2, the IP also accesses a previous-frame buffer through memory-mapped access.
5. The processed stream is sent to HDMI output.

---

## 9. Interface choice: AXI4-Stream or shared memory?
The project will use **both**, but for different reasons:

- **AXI4-Stream** is the main interface for the live video path because it matches the PYNQ video subsystem and supports low-latency real-time processing.
- **Shared memory / external frame buffer** is added only for the previous-frame data required by frame differencing.

So the design is not choosing one model exclusively. It uses a **streaming pipeline with memory-backed state**.

This is a stronger architectural description than saying only “streaming” or only “shared memory.”

---

## 10. HLS design strategy
The implementation will use HLS-friendly practices such as:

- modular functions,
- `hls::stream` between internal stages,
- loop pipelining,
- `DATAFLOW` across independent stages,
- fixed-point or integer arithmetic where practical,
- line buffers / sliding windows for 3x3 filters.

### Candidate top-level structure
```cpp
void video_motion_ip(
    hls::stream<axis_pixel_t> &in_stream,
    hls::stream<axis_pixel_t> &out_stream,
    volatile ap_uint<8> *prev_frame,
    int mode,
    int thresh,
    int motion_thresh,
    int rows,
    int cols
) {
    #pragma HLS INTERFACE axis port=in_stream
    #pragma HLS INTERFACE axis port=out_stream
    #pragma HLS INTERFACE m_axi port=prev_frame offset=slave bundle=GMEM
    #pragma HLS INTERFACE s_axilite port=mode bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=thresh bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=motion_thresh bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=rows bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=cols bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL
    #pragma HLS DATAFLOW

    // Stage 1: input unpack
    // Stage 2: grayscale conversion
    // Stage 3: optional denoise filter
    // Stage 4: threshold visualization (Milestone 1)
    // Stage 5: previous-frame access (Milestone 2)
    // Stage 6: abs difference + motion threshold (Milestone 2)
    // Stage 7: output / overlay formatting
}
```

---

## 11. Verification plan

### 11.1 Functional verification
- Create a C/C++ testbench for small sample frames.
- Compare against a Python or NumPy reference model.
- Verify grayscale, filter, threshold, and motion-mask modes.

### 11.2 Co-simulation and synthesis checks
- Run C simulation and RTL co-simulation in Vitis HLS.
- Check AXI4-Stream sideband handling.
- Inspect initiation interval, resource usage, and latency.

### 11.3 Board-level testing
- Integrate the IP into a PYNQ overlay.
- Test bypass mode first.
- Test grayscale and filtered output.
- Then test motion mask and motion overlay using HDMI input/output.

---

## 12. Stretch goals
If the main pipeline works early, possible extensions include:

- adaptive motion thresholding,
- bounding-box generation for moving regions,
- connected-component labeling,
- background subtraction instead of simple frame differencing,
- support for multiple output highlight colors,
- statistics such as percentage of frame in motion.


---

## 13. References
- PYNQ Video subsystem documentation: https://pynq.readthedocs.io/en/v2.6.1/pynq_libraries/video.html
- AMD Vitis HLS User Guide (UG1399): AXI4-Stream interfaces and `hls::stream`
- AMD Vitis tutorials on line buffers and sliding windows for 2D filtering
