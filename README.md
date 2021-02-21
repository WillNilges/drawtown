# drawtown
Whiteboard -> Minetest interface. Prototype your ideas™

## What is this thing?

DrawTown lets you transfer sketches from whiteboard to the digital realm. It enables you to scan a pic, and send it into a voxel engine called MineTest, where terrain and buildings are generated on the fly for you to explore and tweak. DrawTown is a great way to quickly prototype new worlds, or expand an existing one.

Under the hood, DrawTown uses OpenCV in C++ to scan a whiteboard photo. It then uses tech like contours and Hough circles to interpret markings into shapes. The dimensions of the shapes are sent over a network socket to a server running alongside the Minetest server. Using some cleverly adapted Minetest modding API calls in Lua, the dimension data retrieved from the socket is sent as World Edit commands to alter the landscape in near-real time using Minetest's schematic system. Data sent is interpreted as commands to create and position schematics in the world. No reloading, no relogging, and with just a few strokes of an Expo marker. It's like magic!


### Dependencies:

`dnf install opencv-devel`

or

`apt install opencv-dev`

or

`pacman -S opencv vtk hdf5`

Take your pick.

### Running:

`make && ./drawtown img/circles03.jpg out.log 0.025 && cat out.log`


