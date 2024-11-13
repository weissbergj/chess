The Mandelbrot set is now very fast. This file makes things for the graphical interface. There is a fun Mandelbrot test and the extension draws lines and triangles with color blending.

For extension: 

Without FPU: triangle rendering time = 116665 ms
With FPU: triangle rendering time = 72540 ms
Mandelbrot was a lot faster, although triangle stuff wasn't!

References
RISC-V ISA documentation
Course materials (CSR stuff)
Online graphics
https://www.geeksforgeeks.org/dda-line-generation-algorithm-computer-graphics/
https://mathworld.wolfram.com/MandelbrotSet.html
https://paulbourke.net/fractals/mandelbrot/
https://www.tutorialspoint.com/computer_graphics/line_generation_algorithm.htm
https://www.geeksforgeeks.org/scan-line-polygon-filling-using-opengl-c/
https://en.wikipedia.org/wiki/Scanline_rendering

Line-Drawing
Approach: Digital Differential Analyzer (DDA) algorithm.
Reason: Easy to implement and for arbitrary slopes.
Operation: Calculate dx, dy, determine steps, incrementally plot points from start to end.

Triangle-Drawing
Approach: Scanline filling.
Reason: Efficient for rasterizing filled triangles.
Operation: Draw edges, calculate scanline intersections, fill pixels between intersections.