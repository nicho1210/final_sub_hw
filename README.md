# Real-Time Threshold Video and Motion Detection Reporting IP for PYNQ

## Repository URL
**GitHub repository:** `https://github.com/nicho1210/hd_final.git`

### Notes for grader
This repository contains the **final HLS IP source at the root level** together with a matching header and testbench.  
Please inspect these files first:

- `README.md`
- `video_ip.cpp`
- `video_ip.h`
- `tb_video_ip.cpp`
- `Makefile` (if included)

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
  - the total motion count for the frame, and
  - which of the **9 screen regions** contain motion.

Rather than directly drawing the motion mask on the HDMI output, the final design separates the system into:
- a **stable real-time video path**, and
- a **background motion-analysis path**.

This separation was the key architectural decision that made the final system reliable.

---

## 2. Final project objective

### Intended functionality
The system takes live HDMI input and performs the following in real time:
- receive HDMI video through the PYNQ video pipeline
- convert the image into a thresholded black/white output
- store compact previous-frame state in FPGA memory
- compare current and previous binary frame samples
- count how many motion blocks changed
- determine which of the **3×3 screen regions** contain motion
- send the motion results to the PS and print them through UART

### Final output behavior
- **HDMI output:** thresholded image
- **UART output:** per-frame motion count and active motion regions (1–9)

---

## 3. Repository contents in this submission
The root-level files in this repository are intended to match the final HLS design described in this README.

### Important files
- `video_ip.cpp` — final HLS IP source
- `video_ip.h` — final HLS IP interface definitions
- `tb_video_ip.cpp` — final HLS testbench
- `Makefile` — build/simulation helper if included
- `README.md` — design overview and grading notes

### What this repository is meant to show
This repository is primarily focused on:
- the **final HLS IP**
- the **matching interface definition**
- the **testbench for functional verification**
- the **system-level design explanation**

If additional synthesis reports, timing reports, screenshots, or board-level integration files are added later, they should be referenced from this README in the same style.

---

## 4. Original target and final implementation

### Original target
The original goal was to build a real-time HDMI motion detection IP that could:
- compare the current frame with the previous frame,
- generate a motion mask,
- and overlay or display the motion result directly on the HDMI output.

### Final implementation
The final system provides:
- stable HDMI threshold output,
- background previous-frame motion analysis,
- packed motion reporting,
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

## 5. IP interface definition

### 5.1 Role of the custom IP
The custom IP, implemented as **`video_gray_live`**, sits in the AXI4-Stream video pipeline and performs:
1. real-time threshold preprocessing for HDMI output
2. background previous-frame comparison for motion detection
3. packed motion-result export to the PS through a side-channel signal

### 5.2 Streaming interface
The IP uses:
- **AXI4-Stream input** for incoming video pixels
- **AXI4-Stream output** for outgoing processed video pixels

These ports carry:
- pixel data
- frame-start information (`TUSER`)
- line-end information (`TLAST`)
- valid/ready handshake (`TVALID`, `TREADY`)

### 5.3 Side-channel interface
In the final version, the IP also exports a packed motion-information signal:
- **`motion_count_out[31:0]`**

This signal is intended to connect to **AXI GPIO**, then be read by the PS and printed through UART.

### 5.4 Packed message format
The 32-bit motion information word is defined as:
- **[15:0]** = `motion_count`
- **[24:16]** = `region_mask`
- **[31:25]** = reserved

where:
- `motion_count` = total number of changed sampled blocks in the frame
- `region_mask` = 9-bit flag showing which of the 3×3 screen regions contain motion

### 5.5 Processing-system interaction
The PS is responsible for:
- clock setup
- VTC setup
- reset control
- HDMI lock monitoring
- reading the packed motion information
- UART reporting of motion results

This means the message flow between the IP and the processor is explicitly defined:
- **PL computes motion**
- **side-channel exports summary information**
- **PS reads and reports it**

---

## 6. Final system behavior

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

## 7. Final hardware architecture

### Data path
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

### Side-channel motion reporting path
The HLS IP also outputs a packed motion-information word through a side-channel:

```text
video_gray_live motion_count_out[31:0]
-> AXI GPIO
-> PS software
-> UART printout
```

### Why the extra register slice and FIFO were added
During debugging, we found that the video path needed buffering and stabilization support. We inserted:
- an **AXI4-Stream Register Slice**
- an **AXI4-Stream Data FIFO**

to improve robustness in the HDMI output path.

---

## 8. IP design and module breakdown
The final design can be understood as the following logical modules.

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

### Module F: Side-channel export path
Packs motion results into a 32-bit word and exports them for PS-side reading.

### Module G: PS control and reporting
The PS configures the video pipeline, monitors status, and reports motion results through UART.

---

## 9. Pipelining and throughput strategy

### 9.1 Streaming requirement
The video path must sustain real-time HDMI throughput, so the design is built around a **streaming pixel pipeline**.

### 9.2 HLS strategy
The HLS design uses:
- AXI4-Stream interfaces
- pipelined loop structure
- simple per-pixel arithmetic on the displayed video path
- compact on-chip previous-frame state
- a motion-analysis path that does **not directly control the displayed output pixels**

### 9.3 Critical architectural decision
The most important throughput decision was to **separate displayed video output from previous-frame-driven motion output**.

That is:
- threshold output remains on the stable path
- previous-frame motion logic runs in the background
- only summarized motion information is exported

This was necessary to maintain reliable video throughput.

---

## 10. HLS IP functionality
The final HLS IP performs two tasks at the same time.

### 10.1 Stable video output path
For every incoming pixel:
1. read the input pixel
2. use the green channel as a simple grayscale proxy
3. compare against a threshold
4. output a stable thresholded pixel to HDMI

This part is the displayed video.

### 10.2 Background motion detection path
At the same time, the IP also:
1. samples one point per **4×4 block**
2. converts it into a 1-bit binary value
3. reads the stored previous-frame value for that block
4. computes whether that block changed
5. updates:
   - total motion count
   - region mask (1–9)
6. stores the current block value for the next frame

This part is not drawn on the video output. Instead, it is exported through the side-channel and then reported by software.

---

## 11. Motion detection algorithm

### 11.1 Block-based sampling
Instead of comparing every pixel, the image is sampled once per **4×4 block**.

For a 1280×720 frame:
- horizontal samples = 1280 / 4 = 320
- vertical samples = 720 / 4 = 180

So the motion detector compares:

320 × 180 = 57600

sample blocks per frame.

### 11.2 Binary thresholding
For each sampled block location:

- `curr_bin = 1` if `G >= T`
- `curr_bin = 0` otherwise

where:
- `G` is the selected pixel intensity (green channel)
- `T` is the threshold

### 11.3 Previous-frame comparison
Let:
- `curr_bin` = current frame binary value
- `prev_bin` = previous frame binary value stored in on-chip memory

Then motion is computed as:

- `motion = curr_bin XOR prev_bin`

So:
- if the block changed between frames, motion = 1
- otherwise, motion = 0

### 11.4 Motion count
If `motion = 1`, then `motion_count` is incremented.

This gives the total number of changed sampled blocks in the frame.

### 11.5 Region detection (1–9)
The screen is divided into a 3×3 grid:

```text
1 2 3
4 5 6
7 8 9
```

If motion occurs in a block inside a region, that region’s bit is set in a 9-bit mask.

Thus, the system reports:
- how much motion occurred
- and where it occurred

---

## 12. Verification and evaluation

### 12.1 Functional checks
The intended functional checks for the final design are:
- threshold output correctness
- sideband correctness (`TUSER`, `TLAST`)
- previous-frame motion accumulation
- `motion_count` correctness
- `region_mask` correctness

### 12.2 Testbench
The repository includes a C++ testbench for the HLS IP.

The final testbench should check:
- thresholded output pixels
- packed motion count
- packed region mask

### 12.3 Hardware checks
Board-level checks used during development included:
- UART logs
- RX/TX lock monitoring
- ILA probes
- timing reports
- block design validation

### 12.4 Final validation goals
Final success criteria:
- HDMI threshold output stable
- `rx_lock = 1`
- `tx_lock = 1`
- motion count changes when the scene changes
- region outputs track where motion occurs

### 12.5 Simulation and synthesis evidence
For grading, this repository should ideally include:
- HLS C simulation output
- synthesis report summaries
- latency / throughput summary tables
- resource utilization tables

A useful summary table format is:

| Version | Description | II | Estimated Clock | LUT | FF | BRAM | DSP | Result |
|---|---|---:|---:|---:|---:|---:|---:|---|
| v1 | Pass-through | fill in | fill in | fill in | fill in | fill in | fill in | Stable |
| v2 | Threshold | fill in | fill in | fill in | fill in | fill in | fill in | Stable |
| v3 | Motion output attempt | fill in | fill in | fill in | fill in | fill in | fill in | Unstable |
| vFinal | Threshold + UART motion report | fill in | fill in | fill in | fill in | fill in | fill in | Stable |

### 12.6 Comparison against initial goals
The initial goal was direct motion-mask or overlay output on HDMI.  
The final evaluation showed that this was not stable in the current architecture.

The final implementation instead met the following practical goals:
- stable real-time HDMI output
- real-time previous-frame motion analysis
- live UART reporting of motion magnitude and motion location

---

## 13. Final design summary

### What works now
- Real-time HDMI input
- Stable HDMI threshold output
- Background previous-frame motion detection
- Motion count per frame
- 1–9 region detection
- Motion summary export for PS/UART reporting

### What does not yet work reliably
- Direct HDMI motion-mask output
- Direct overlay of previous-frame based motion on the displayed video path

---

## 14. Software role (PS side)
In the final board-level system, the PS software performs:
- clock wizard setup
- VTC generator setup
- reset control
- HDMI RX/TX lock monitoring
- reading packed motion information
- UART reporting of:
  - motion count
  - motion detected / not detected
  - active 1–9 regions

The PS does **not** process the video stream itself. All video processing is done in programmable logic.

---

## 15. Organization and documentation notes

### 15.1 Keep the repository consistent
The most important rule for this submission is that:
- `README.md`
- `video_ip.cpp`
- `video_ip.h`
- `tb_video_ip.cpp`

must all describe the **same final design**.

### 15.2 Keep the repo clean
Avoid committing unnecessary large generated files such as:
- `.jou`
- `.log`
- `.cache`
- temporary run directories if not needed
- very large duplicated build outputs

### 15.3 Make evidence easy to inspect
Because the grader will not execute the code, documentation should clearly point to:
- custom IP source file
- testbench
- synthesis summaries
- timing summaries
- architecture diagrams
- UART output examples

---

## 16. Current limitations
- Motion results are block-based, not full pixel-accurate masks
- No HDMI motion-mask overlay in the final stable version
- Current threshold uses a simple single-channel comparison
- No advanced cleanup or filtering is applied to the motion mask yet

---

## 17. Future work
Possible next steps include:
- frame-buffer based motion-mask overlay using DDR / VDMA
- region-wise motion thresholds
- bounding boxes for moving regions
- temporal filtering for more stable motion decisions
- adaptive thresholding
- object-level motion tracking

---

## 18. Final takeaway
This project showed that in FPGA video systems, the challenge is not only computing the algorithm correctly, but also preserving a **stable real-time output path**.

The final implementation successfully balances both:
- **stable live video output**
- and **real-time motion detection reporting**

---

## 19. References
- PYNQ Video subsystem documentation
- AMD Vitis HLS User Guide (UG1399)
- AMD Vivado AXI4-Stream video IP documentation
- PYNQ-Z2 HDMI reference pipeline resources
