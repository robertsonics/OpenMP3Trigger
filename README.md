# OpenMP3Trigger

Open Source MP3 Player firmware for the Robertsonics/SparkFun WAV Trigger board

This project is implemented in C using the STM32CubeMX and the STM32 Extension with Visual Studio Code, and is designed to run on a Robertsonics/SparkFun **WAV Trigger** board. It improves upon the
features of the original **MP3 Trigger** which is based on the VLSI VS1063 MP3 decoder IC and is limited to a single MP3 stream at a time, preventing crossfading and mixing of tracks.

This project will demonstrate some of my realtime, fixed-point audio programming methods, used in the WAV Trigger, Tsunami and WAV Trigger Pro products, for Arm Cortex M4, M7 and H7 processors. In the process, I hope to offer an open-source MP3 player platform that provides better capabilities and more customization options than is currently available.

Like all Robertsonics players, this code is optimized for streaming audio from microSD. It is a dedicated audio player, not a library function to be added to a general purpose Arduino or Raspberry Pi while doing other things. Playing and mixing glitch-free audio is prioritized. Audio can be controlled via 16 digital input triggers or through a serial ASCII command-line interface. As an open-source project, you are free to add whatever additional control capabilities you want, but you must keep in mind that there's a lot of time-sensitive stuff being coordinated to prevent glitches.

As of 5/21/2025 this code is not yet functional - I just wanted to get a first commit up and start reporting my progress. Stay tuned...
