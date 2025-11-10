# <img src="./res/icon.png" width="26" height="26"/> LithoGen

A cross-platform lithophane generator with 3D preview and in-depth configuration written in C++ with efficiency,
customizability and usability in mind. Supports loading images of types `jpeg`, `png`, `tga`, `bmp`, `psd`, `gif`,
`hdr`, `pic` and exports to `stl`.

![Application Preview](./res/preview.png)

## Installation

Windows users can download the latest binary from
the [GitHub Releases](https://github.com/YellowAtom/lithogen/releases), otherwise it must be built from source.

### Building From Source

The project uses the CMake build system with all dependencies either bundled or automatically downloaded. Therefore,
CMake can be used as normal.

## Dependencies

The official name, source and version of the project dependencies.

| Name                        | Version        | Source                                                                            | 
|-----------------------------|----------------|-----------------------------------------------------------------------------------|
| Battery Embed               | 1.2.19         | https://github.com/batterycenter/embed                                            |
| Glad                        | 2.0.8          | https://github.com/Dav1dde/glad                                                   |
| GLFW                        | 3.4            | https://github.com/glfw/glfw                                                      |
| GLM                         | 1.0.1          | https://github.com/g-truc/glm                                                     |
| Dear ImGui                  | 1.91.8         | https://github.com/ocornut/imgui                                                  |
| microstl                    | Commit ec3868a | https://github.com/cry-inc/microstl/tree/ec3868a14d8eff40f7945b39758edf623f609b6f |
| Native File Dialog Extended | 1.2.1          | https://github.com/btzy/nativefiledialog-extended                                 |
| std_image                   | 2.30           | https://github.com/nothings/stb                                                   |
