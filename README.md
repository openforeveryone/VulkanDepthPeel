# Vulkan depth peel demo
Demonstrates order independent transparency on Vulkan using depth peeling. This code accompanies the blog post [Depth Peeling Order Independent Transparency in Vulkan] (https://matthewwellings.com/blog/depth-peeling-order-independent-transparency-in-vulkan/).

This demo will run on Linux (XCB) and Android. It makes use of subpasses, input attachments and reusable command buffers.

Keys (on Linux):
- Space to toggle split-screen (left is traditional order dependent right is depth peeled)
- Up and down to change number of layers used.
- Left and right to change number of objects rendered.
- W and S to display only one of the peeled layers and to select the currently displayed layer.

![Screenshot](https://github.com/openforeveryone/VulkanDepthPeel/blob/master/ScreenShot.png "Screenshot")

All blocks are the same size and rendered in arbitrary order in separate draw calls.
