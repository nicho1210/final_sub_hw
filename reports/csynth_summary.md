# HLS C Synthesis Summary

## Final design
**Top function:** `video_gray_live`

## Pipeline / performance summary

| Metric | Value |
|---|---:|
| Pipelined | Yes |
| Initiation Interval (II) | 1 |
| Iteration Latency | 2 cycles |
| Trip Count | inf |

## Resource utilization

| Resource | Usage |
|---|---:|
| BRAM | 0 |
| DSP | 0 |
| FF | 180 |
| LUT | 1591 |
| URAM | 0 |

## Interpretation
The final HLS design achieved a **pipelined implementation with II = 1**, which matches the intended real-time streaming goal. The synthesized design uses **1591 LUTs** and **180 FFs**, with **0 BRAM**, **0 DSP**, and **0 URAM** reported in this summary view.