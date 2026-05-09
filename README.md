## Repository URL
**GitHub repository:** `https://github.com/nicho1210/hardware_upload_try`

### Notes for grader
This repository contains the **final root-level HLS design files** for grading.  
Please inspect these files first:

- `README.md`
- `video_ip.cpp` — final HLS IP source
- `video_ip.h` — final HLS IP interface definition
- `tb_video_ip.cpp` — final HLS testbench
- `Makefile` — build/simulation helper if included

The board-level system described here is the final PYNQ-Z2 HDMI design used in development:
- HDMI input
- custom HLS IP in the AXI4-Stream video path
- stable thresholded HDMI output
- packed motion summary sent to PS/UART

---

## Project title
**Real-Time Threshold Video and Motion Detection Reporting IP for PYNQ-Z2**

---

## Project team
- Sora Kakigi
- Thinh Nguyen
- Yi Chieh Huang
- Justin Lu

---

## Repository contents in this submission
The root-level files in this repository are intended to match the final design described in this README.

### Important files
- `video_ip.cpp` — final HLS IP source
- `video_ip.h` — final HLS IP interface definition
- `tb_video_ip.cpp` — final HLS testbench
- `Makefile` — build/simulation helper
- `README.md` — design overview and grading notes

## 1. Project overview
This project implements a **custom Vitis HLS video IP** for the PYNQ-Z2 HDMI pipeline. The final system accepts a live HDMI video stream, performs **real-time threshold-based video preprocessing** on the FPGA, and computes **block-based motion detection** in the background by comparing the current frame against the previous frame.

The final design outputs:

- a **stable thresholded video** on HDMI output, and
- **motion detection results through UART**, including:
  - total motion count for the frame,
  - and which of the **9 screen regions** contain motion.

Rather than directly drawing the motion mask on the HDMI output, the design is organized into:
- a **stable real-time video path**, and
- a **background motion-analysis path**.

This separation was the key architectural decision that made the final system reliable.

---

## 2. Final project objective

### Intended functionality
The system takes live HDMI input and performs the following in real time:

- Receive HDMI video through the PYNQ video pipeline
- Convert the image into a thresholded black/white output
- Store compact previous-frame state in FPGA memory
- Compare current and previous binary frame samples
- Count how many motion blocks changed
- Determine which of the **3×3 screen regions** contain motion
- Send the motion results to the PS and print them through UART

### Final output behavior
- **HDMI output:** thresholded image
- **UART output:** per-frame motion count and active motion regions (1–9)

---

## 3. Original target and final implementation

### Original target
The original goal was to build a real-time HDMI motion detection IP that could:
- compare the current frame with the previous frame,
- generate a motion mask,
- and overlay or display the motion result directly on the HDMI output.

### Final implementation
The final system successfully provides:
- stable HDMI threshold output,
- background previous-frame motion analysis,
- UART motion reporting,
- and 1–9 region-based motion localization.

### Important design lesson
During development, we found that:
- **threshold output was stable**
- **previous-frame memory access was possible**
- but **when previous-frame data directly affected the video output path, the HDMI output became unstable**

Therefore, the final design uses:
- **threshold video for display**
- **motion detection for reporting**

This provides a robust and repeatable real-time FPGA implementation.

---

## 4. IP interface definition

### 4.1 Role of the custom IP
The custom IP, referred to in this project as **`video_gray_live`**, sits in the AXI4-Stream video pipeline and performs:

1. real-time threshold preprocessing for HDMI output
2. background previous-frame comparison for motion detection
3. packed motion-result export to the PS through a GPIO-visible output signal

### 4.2 Streaming interface
The IP uses:
- **AXI4-Stream input** for incoming video pixels
- **AXI4-Stream output** for outgoing processed video pixels

These ports carry:
- pixel data
- frame-start information (`TUSER`)
- line-end information (`TLAST`)
- valid/ready handshake (`TVALID`, `TREADY`)

### 4.3 Side-channel interface
In the final version, the IP also exports a packed motion-information signal:

- **`motion_info_out[31:0]`**

This signal is connected to **AXI GPIO**, then read by the PS and printed through UART.

### 4.4 Packed message format
The 32-bit motion information word is defined as:

- **[15:0]** = `motion_count`
- **[24:16]** = `region_mask`
- **[31:25]** = reserved

where:
- `motion_count` = total number of changed sampled blocks in the frame
- `region_mask` = 9-bit flag showing which of the 3×3 screen regions contain motion

### 4.5 Processing-system interaction
The PS is responsible for:
- clock setup
- VTC setup
- reset control
- HDMI lock monitoring
- reading `motion_info_out` via AXI GPIO
- UART reporting of motion results

This means the message flow between the IP and the processor is explicitly defined:
- **PL computes motion**
- **AXI GPIO transfers summary information**
- **PS reads and reports it**

---

## 5. Final system behavior

### HDMI output
The HDMI output shows a **thresholded binary image**:
- white pixels for values above threshold
- black pixels for values below threshold

### UART output
The UART prints:
- whether the HDMI RX and TX are locked,
- the motion count for the current frame,
- whether motion is detected,
- and which screen regions contain motion.

Example:

```text
[HB] app alive=12, rx_lock=1, tx_lock=1, motion_count=428, motion=1, regions=2,5,9
```

This means:
- the video system is alive,
- input and output are locked,
- 428 motion blocks changed,
- and motion was detected in regions 2, 5, and 9.

---

## 6. Final hardware architecture

### 6.1 Data path
The final hardware video path is:

```text
HDMI Input
-> v_vid_in_axi4s
-> video_gray_live (custom HLS IP)
-> AXI4-Stream Register Slice
-> AXI4-Stream Data FIFO
-> v_axi4s_vid_out
-> HDMI Output
```

### 6.2 Side-channel motion reporting path
The HLS IP also outputs a packed motion-information word to AXI GPIO:

```text
video_gray_live motion_info_out[31:0]
-> AXI GPIO
-> PS software
-> UART printout
```

### 6.3 Why the extra register slice and FIFO were added
During debugging, we found that the video path needed buffering and stabilization support. We inserted:
- an **AXI4-Stream Register Slice**
- an **AXI4-Stream Data FIFO**

to improve robustness in the HDMI output path.

---

## 7. IP design and module breakdown

The final design can be understood as the following logical modules:

### Module A: HDMI input and AXI4-Stream conversion
Receives live HDMI video and converts it into an AXI4-Stream video interface compatible with the custom HLS IP.

### Module B: Threshold video generation
Inside the HLS IP, the incoming video is reduced to a binary thresholded output for stable display.

### Module C: Previous-frame memory
Stores one binary state per 4×4 block to represent the previous frame.

### Module D: Motion comparison engine
Compares current sampled binary block state with previous-frame binary state using XOR.

### Module E: Motion summary logic
Accumulates:
- frame-level motion count
- 3×3 region mask

### Module F: AXI GPIO export path
Packs motion results into a 32-bit word and exports them to PS-visible AXI GPIO.

### Module G: PS control and reporting
The PS configures the video pipeline, monitors status, and reports motion results through UART.

---

## 8. Pipelining and throughput strategy

### 8.1 Streaming requirement
The video path must sustain real-time HDMI throughput, so the design is built around a **streaming pixel pipeline**.

### 8.2 HLS strategy
The HLS design uses:
- AXI4-Stream interfaces
- pipelined loop structure
- simple per-pixel arithmetic on the displayed video path
- compact on-chip previous-frame state
- a motion-analysis path that does **not directly control the displayed output pixels**

### 8.3 Critical architectural decision
The most important pipelining decision was to **separate displayed video output from previous-frame-driven motion output**.

That is:
- threshold output remains on the stable path
- previous-frame motion logic runs in the background
- only summarized motion information is exported

This was necessary to maintain reliable video throughput.

---

## 9. HLS IP functionality

The final HLS IP performs two tasks at the same time.

### 9.1 Stable video output path
For every incoming pixel:
1. Read RGB input pixel
2. Use the green channel as a simple grayscale proxy
3. Compare against a threshold
4. Output a stable thresholded pixel to HDMI

This part is the displayed video.

### 9.2 Background motion detection path
At the same time, the IP also:
1. Samples one point per **4×4 block**
2. Converts it into a 1-bit binary value
3. Reads the stored previous-frame value for that block
4. Computes whether that block changed
5. Updates:
   - total motion count
   - region mask (1–9)
6. Stores the current block value for the next frame

This part is not drawn on the video output. Instead, it is reported to the processor through AXI GPIO and then printed to UART.

---

## 10. Motion detection algorithm

### 10.1 Block-based sampling
Instead of comparing every pixel, the image is sampled once per **4×4 block**.

For a 1280×720 frame:
- horizontal samples = 1280 / 4 = 320
- vertical samples = 720 / 4 = 180

So the motion detector compares:

\[
320 	imes 180 = 57600
\]

sample blocks per frame.

### 10.2 Binary thresholding
For each sampled block location, we compute:

\[
curr\_bin =
Begin{cases}
1 & 	ext{if } G \ge T \
0 & 	ext{otherwise}
\end{cases}
\]

where:
- `G` is the selected pixel intensity (green channel),
- `T` is the threshold.

### 10.3 Previous-frame comparison
Let:
- `curr_bin` = current frame binary value
- `prev_bin` = previous frame binary value stored in on-chip memory

Then motion is computed as:

\[
motion = curr\_bin \oplus prev\_bin
\]

So:
- if the block changed between frames, motion = 1
- otherwise, motion = 0

### 10.4 Motion count
If `motion = 1`, then:

\[
motion\_count = motion\_count + 1
\]

This gives the total number of changed sampled blocks in the frame.

### 10.5 Region detection (1–9)
The screen is divided into a 3×3 grid:

```text
1 2 3
4 5 6
7 8 9
```

If motion occurs in a block inside a region, that region’s bit is set in a 9-bit mask.

Thus, the system reports:
- how much motion occurred,
- and where it occurred.

---

## 11. Packed motion output format
The HLS IP outputs a 32-bit word to AXI GPIO:

- **[15:0]** = `motion_count`
- **[24:16]** = `region_mask`
- **[31:25]** = reserved

This is then read by the PS and decoded for UART output.

---

## 12. Why this project was challenging
This project turned out to be more about **stable real-time video architecture** than just image-processing math.

The main difficulty was not computing motion itself, but maintaining a valid HDMI video stream while adding previous-frame logic.

We found that:
- grayscale worked,
- threshold worked,
- memory access worked,
- previous-frame read/write worked in the background,

but:
- once previous-frame data directly controlled the output pixel path,
- the HDMI output became unstable and `tx_lock` failed.

This forced us to redesign the architecture so that motion analysis happened in the background while the displayed video remained on a stable threshold path.

---

## 13. Major debugging steps and challenges

### 13.1 HDMI pass-through bring-up
We first verified:
- VTC configuration
- clock wizard configuration
- reset sequence
- HDMI RX and TX lock behavior

This established a working baseline.

### 13.2 Grayscale and threshold validation
We inserted our HLS IP into the pipeline and confirmed:
- grayscale output worked
- threshold output worked
- AXI4-Stream video integration was correct

### 13.3 Motion-output failures
When we first tried to directly output motion-mask results, the system failed with:
- `rx_lock = 1`
- `tx_lock = 0`

### 13.4 AXI stream debugging
We used:
- UART logs
- multiple ILA probes
- AXI FIFO
- AXI register slice

to verify:
- `TUSER`
- `TLAST`
- `TDATA`
- `TVALID`
- `TREADY`

These experiments showed that the stream protocol looked valid, but the video path still became unstable for motion-output versions.

### 13.5 Timing issue
At one point, implementation timing failed due to heavy debug instrumentation. After removing extra ILAs, timing passed again.

### 13.6 Isolating the root cause
We then ran controlled HLS experiments and discovered:

- Threshold output + previous-frame write: **works**
- Threshold output + previous-frame read/write in background: **works**
- Output directly depends on previous-frame value: **fails**
- Output directly depends on frame-difference result: **fails**

This led to the final solution:
- keep displayed video simple and stable,
- keep motion detection in background,
- report results through UART.

---

## 14. Verification and evaluation

### 14.1 Functional checks
We incrementally verified:
- pass-through
- grayscale
- threshold
- previous-frame background access
- motion counting
- region reporting

### 14.2 Hardware checks
We used:
- UART logs
- RX/TX lock monitoring
- ILA probes
- timing reports
- block design validation

### 14.3 Final validation goals
Final success criteria:
- HDMI threshold output stable
- `rx_lock = 1`
- `tx_lock = 1`
- motion count changes when the scene changes
- region outputs track where motion occurs

### 14.4 Simulation and synthesis evidence
To make the repository grading-friendly, include evidence such as:
- HLS C simulation screenshots or notes
- synthesis report summaries
- latency / throughput summary tables
- resource utilization tables

Recommended items to add to the repository:
- a short section summarizing **loop II**
- estimated latency or throughput from HLS
- LUT / FF / BRAM / DSP usage
- screenshots or copied tables from synthesis reports

A suggested summary table format is:

| Version | Description | II | Estimated Clock | LUT | FF | BRAM | DSP | Result |
|---|---|---:|---:|---:|---:|---:|---:|---|
| v1 | Pass-through | TODO | TODO | TODO | TODO | TODO | TODO | Stable |
| v2 | Threshold | TODO | TODO | TODO | TODO | TODO | TODO | Stable |
| v3 | Motion output attempt | TODO | TODO | TODO | TODO | TODO | TODO | Unstable |
| vFinal | Threshold + UART motion report | TODO | TODO | TODO | TODO | TODO | TODO | Stable |

> Replace `TODO` with actual values from your reports before grading.

### 14.5 Comparison against initial goals
The initial goal was direct motion-mask or overlay output on HDMI.  
The final evaluation showed that this was not stable in the current architecture.

The final implementation instead met the following practical goals:
- stable real-time HDMI output
- real-time previous-frame motion analysis
- live UART reporting of motion magnitude and motion location

---

## 15. Final design summary

### What works now
- Real-time HDMI input
- Stable HDMI threshold output
- Background previous-frame motion detection
- Motion count per frame
- 1–9 region detection
- UART motion reporting

### What does not yet work reliably
- Direct HDMI motion-mask output
- Direct overlay of previous-frame based motion on the displayed video path

---

## 16. Final system capability
The final system performs:

- real-time frame-to-frame comparison,
- block-based motion detection,
- spatial region classification,
- and live motion reporting through UART,

while maintaining a stable HDMI threshold output.

A precise description of the completed system is:

> A real-time block-based motion detection system with stable threshold video output and UART-based motion reporting.

---

## 17. Software role (PS side)
The PS software performs:
- clock wizard setup
- VTC generator setup
- reset control
- HDMI RX/TX lock monitoring
- AXI GPIO reads of packed motion information
- UART reporting of:
  - motion count
  - motion detected / not detected
  - active 1–9 regions

The PS does **not** process the video stream itself. All video processing is done in programmable logic.

---

## 18. HLS design strategy
The HLS implementation follows these principles:

- streaming AXI4-Stream input/output
- one-pixel-per-cycle pipeline target
- lightweight arithmetic for the displayed video path
- compact on-chip previous-frame state
- background motion analysis
- output/result separation for stability

---

## 19. Organization and documentation notes

### 19.1 Recommended repository structure
To make the repository easier for a grader to inspect, organize it so that the design flow is clear:

- root `README.md`
- HLS source folder
- Vivado integration folder
- software/UART control folder
- reports or report summaries
- optional images/screenshots folder

### 19.2 Keep the repo clean
Avoid committing unnecessary large generated files such as:
- `.jou`
- `.log`
- `.cache`
- temporary run directories if not needed
- very large duplicated build outputs

It is fine to include selected exported design files or reports if they are directly useful for grading.

### 19.3 Make evidence easy to inspect
Because the grader will not execute the code, documentation should clearly point to:
- custom IP source file
- testbench or simulation notes
- synthesis summaries
- implementation/timing summaries
- final architecture diagram
- UART output examples

### 19.4 Suggested “files to inspect” note
It is helpful to add a short section near the top of the repo like:

- `README.md` for project summary
- `hls_src/` for custom IP implementation
- `reports/` for synthesis and timing summaries
- `sw/` for PS software and UART reporting
- `images/` for screenshots of output and debug results

---

## 20. Current limitations
- Motion results are block-based, not full pixel-accurate masks
- No HDMI motion-mask overlay in the final stable version
- Current threshold uses a simple single-channel comparison
- No advanced cleanup/filtering is applied to the motion mask yet

---

## 21. Future work
Possible next steps include:
- frame-buffer based motion-mask overlay using DDR / VDMA
- region-wise motion thresholds
- bounding boxes for moving regions
- temporal filtering for more stable motion decisions
- adaptive thresholding
- object-level motion tracking

---

## 22. Final takeaway
This project showed that in FPGA video systems, the challenge is not only computing the algorithm correctly, but also preserving a **stable real-time output path**.

The final implementation successfully balances both:
- **stable live video output**
- and **real-time motion detection reporting**

---

## 23. References
- PYNQ Video subsystem documentation
- AMD Vitis HLS User Guide (UG1399)
- AMD Vivado AXI4-Stream video IP documentation
- PYNQ-Z2 HDMI reference pipeline resources

## Important files and paths
- HLS IP source: `path/to/video_gray_live.cpp`
- PS software: `path/to/video_out_pynq.c`
- Hardware block design export: `path/to/bd.tcl`
- HLS synthesis report: `path/to/csynth.rpt`
- Timing / implementation summary: `path/to/timing_summary.txt`