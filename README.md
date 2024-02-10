# DXGI_Format Support Exporter (DFSE)
 A handy dandy tool for viewing and dumping DXGI Format support. Made with ImGui and DirectX 12.
 
 A bit like this [Vulkan web tool](https://vulkan.gpuinfo.org/) maintained by the fantastic Sascha Willems
 
 *Requires DirectX 12 support (FL 11_0 minimum)*

 ### Motivation:
 Not every GPU supports every format and can be used in every case. AMD GPUs often don't support the common D24_S8_UNORM format.
 This tool aims to eliminate these problems by allowing a user to view & dump all formats their GPU supports to a file.
 A developer can then check this list and go "Ohh they don't support --- format" 
![image](https://github.com/Joe-Bevan/DXGI_Format-Support-Exporter/assets/44583084/3b334e83-d687-45b7-9fc6-545101359363)

### TODO list:
- [x] ~Export formats and their support flags to a file~
    - [ ] Export additional relevant GPU info (vendor, model, etc.)
- [ ] Open an exported list to view it easier than reading black and white plain text.  
- [ ] Select a GPU from a list
- [ ] Add/remove formats to the common DXGI formats list.
