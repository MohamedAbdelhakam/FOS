# FOS Operating System Template for Ain Shams University

## Overview

This repository contains the template implementation of FOS (Faculty Operating System), an educational operating system developed by Computer Science students at Ain Shams University. FOS provides a minimal yet functional kernel designed to help students understand core OS concepts through hands‑on experience.

## Key Features

- **Memory Management**
  here some functions examble
  - `kmalloc(size_t size)`: Allocate a contiguous block of kernel memory.
  - `krealloc(void *ptr, size_t new_size)`: Resize an existing kernel memory block.
  - `kfree(void *ptr)`: Free a previously allocated kernel memory block.
  - ..etc
- **Fault Handling**

  - Page fault and general protection fault handlers to gracefully catch and report kernel‐level errors.
  - Customizable exception vectors for implementing additional traps and interrupts.

- **CPU Scheduling**

  - Multi-Level Feedback Queue (MLFQ) scheduler:
    - Dynamic priority adjustment based on CPU bursts.
    - Multiple ready queues with aging to prevent starvation.
    - Time‑slice based preemption.
