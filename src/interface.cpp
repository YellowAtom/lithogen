#include "interface.h"
#include <algorithm>
#include <glad/gl.h>
#include <iostream>
#include <numeric>
#include <stb_image.h>
#include "compilation.h"
#include "constants.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "nfd_glfw3.h"
#include "render.h"

// A simple function to send the image data to the gpu and return the pointer.
GLuint LoadTexture(const stbi_uc* image, const int width, const int height) {
	GLuint imageTexture = 0;
	glGenTextures(1, &imageTexture);
	glBindTexture(GL_TEXTURE_2D, imageTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	return imageTexture;
}

void ImportButton(GLFWwindow* window, Image& image, Config* config) {
	constexpr nfdu8filteritem_t filters[1] = {
		{"Image", "jpg,jpeg,png,tga,bmp,psd,gif,hdr,pic"},
	};

	nfdopendialogu8args_t args = {};
	args.filterList = filters;
	args.filterCount = 1;

	NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);

	nfdu8char_t* outPath = nullptr;

	if (const nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args); result != NFD_OKAY) {
		if (result == NFD_ERROR) {
			std::cout << "Error: " << NFD_GetError() << '\n';
		}

		return;
	}

	if (image.data != nullptr) {
		stbi_image_free(image.data);
	}

	// Load the image from storage, it will automatically process any of the supported formats.
	image.data = stbi_load(outPath, &image.width, &image.height, nullptr, 4);

	if (image.data == nullptr) {
		std::cout << "Failed to load image!\n";
		return;
	}

	// Calculate the information required to gather aspect ratio based sizing.
	const int aspectGcd = std::gcd(image.width, image.height);
	image.aspectRatioW = image.width / aspectGcd;
	image.aspectRatioH = image.height / aspectGcd;

	// Ensure the default width is the correct aspect ratio and reset the size when a new image is
	// loaded.
	config->sliderHeight = 100.0F;
	config->sliderWidth = 100.0F * image.aspectRatioW / image.aspectRatioH;

	if (image.texture != 0) {
		glDeleteTextures(1, &image.texture); // Ensure the previous image has been cleaned up.
	}

	// Send the image to the GPU for the renderer to preview.
	image.texture = LoadTexture(image.data, image.width, image.height);
	// Clear the file path from memory as we are done with it.
	NFD_FreePathU8(outPath);
}

void ExportButton(GLFWwindow* window, const Model& model) {
	if (model.indices.empty()) {
		return;
	}

	constexpr nfdu8filteritem_t filters[1] = {
		{"Binary STL", "stl"},
	};

	nfdsavedialogu8args_t args = {};
	args.filterList = filters;
	args.filterCount = 1;
	args.defaultName = "lithophane";

	NFD_GetNativeWindowFromGLFWWindow(window, &args.parentWindow);

	nfdu8char_t* outPath = nullptr;

	if (const nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args); result != NFD_OKAY) {
		if (result == NFD_ERROR) {
			std::cout << "Error: " << NFD_GetError() << '\n';
		}

		return;
	}

	WriteModel(outPath, model);
	NFD_FreePathU8(outPath);
}

void RenderInterface(GLFWwindow* window, Image& image, Config* config, Model& model, Render* render) {
	// Menu bar
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Import")) {
				ImportButton(window, image, config);
			}
			if (ImGui::MenuItem("Export")) {
				ExportButton(window, model);
			}
			if (ImGui::MenuItem("Quit", "Alt+F4")) {
				glfwSetWindowShouldClose(window, GL_TRUE);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Show Source", nullptr, &config->drawSource);
			ImGui::Separator();
			ImGui::MenuItem("Show Preview", nullptr, &config->drawPreview);
			if (ImGui::MenuItem("Wireframe Preview", "W", &config->drawWireframe)) {
				render->UpdateWireframe();
			}
			if (ImGui::MenuItem("Reset Preview Rotation", "R")) {
				render->camera.ResetRotation();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {
			if (ImGui::MenuItem("About")) {
				config->aboutOpened = true;
			}
			if (ImGui::MenuItem("Help")) {
				config->helpOpened = true;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Gather the window height.
	int width = 0;
	int height = 0;
	glfwGetWindowSize(window, &width, &height);

	// Push the sidepanel down to make room for the menu bar.
	ImGui::SetNextWindowPos(ImVec2(0, GUI_MENUBAR_HEIGHT));
	ImGui::SetNextWindowSize(ImVec2(GUI_SIDEPANEL_WIDTH, height - GUI_MENUBAR_HEIGHT));

	ImGui::Begin("SidePanel", nullptr,
	             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
	                 ImGuiWindowFlags_NoTitleBar);

	if (image.data == nullptr) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5F);
	}

	ImGui::SeparatorText("Mesh Configuration");

	ImGui::Combo("Mesh Type", &config->dropdownMesh, config->dropdownMeshTypes,
	             IM_ARRAYSIZE(config->dropdownMeshTypes));

	ImGui::Text("Dimensions");

	// TODO: The forced ratio does not clamp.
	if (ImGui::SliderFloat("Width", &config->sliderWidth, SLIDER_WIDTH_MIN, SLIDER_WIDTH_MAX, SLIDER_FLOAT_FORMAT_MM)) {
		config->sliderHeight = config->sliderWidth * image.aspectRatioH / image.aspectRatioW;
	}

	if (ImGui::SliderFloat("Height", &config->sliderHeight, SLIDER_HEIGHT_MIN, SLIDER_HEIGHT_MAX,
	                       SLIDER_FLOAT_FORMAT_MM)) {
		config->sliderWidth = config->sliderHeight * image.aspectRatioW / image.aspectRatioH;
	}

	ImGui::Text("Thickness");

	if (ImGui::SliderFloat("Min", &config->sliderThickMin, SLIDER_THICK_MIN, SLIDER_THICK_MAX,
	                       SLIDER_FLOAT_FORMAT_MM)) {
		config->sliderThickMax = std::max(config->sliderThickMin, config->sliderThickMax);
	}

	if (ImGui::SliderFloat("Max", &config->sliderThickMax, SLIDER_THICK_MIN, SLIDER_THICK_MAX,
	                       SLIDER_FLOAT_FORMAT_MM)) {
		config->sliderThickMin = std::min(config->sliderThickMax, config->sliderThickMin);
	}

	ImGui::SeparatorText("Image Processing");

	// TODO: Implement difference kinds of grayscale processing. Currently we are only doing luminance.
	ImGui::Text("Grayscale Preference");
	ImGui::SliderFloat("Red", &config->sliderGsPref[0], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Green", &config->sliderGsPref[1], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
	                   ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Blue", &config->sliderGsPref[2], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT, ImGuiSliderFlags_AlwaysClamp);

	/* ImGui::Text("Alpha Thickness");
	ImGui::SliderFloat("Alpha", &config->sliderGsPref[3], 0.0F, 1.0F, SLIDER_FLOAT_FORMAT,
	                   ImGuiSliderFlags_AlwaysClamp); */

	ImGui::Spacing();

	if (ImGui::Button("Compile")) {
		// TODO: Add some visual indicator that the process is on going.
		// Ideally place the compile processes onto a different thread so some sort of simple animation can play on the
		// loading popup to indicate it has not crashed.
		CompileModel(model, config, image);
		render->entity.LoadModel(model);

		// Offset the position by the centre offset.
		render->entity.SetPosition(-model.centerOffset);

		// Adjust the zoom to focus on the mesh based on the size of it.
		render->camera.SetZoom(std::max(config->sliderWidth, config->sliderHeight) / 1.5F);

		// TODO: Add visual error if compile fails.
	}

	if (image.data == nullptr) {
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::End();

	if (config->drawSource && image.texture != 0) {
		// Place the source preview in the bottom right corner.
		// Add 10 pixels of padding to the window edge.
		float previewWidth = 128.0F;
		float previewHeight = 128.0F;

		// Scale around the larger side.
		if (image.aspectRatioW < image.aspectRatioH) {
			previewHeight = 128.0F * image.aspectRatioH / image.aspectRatioW;
		} else {
			previewWidth = 128.0F * image.aspectRatioW / image.aspectRatioH;
		}

		ImGui::SetNextWindowPos(ImVec2(width - previewWidth - 10.0F, height - previewHeight - 10.0F));
		ImGui::SetNextWindowSize(ImVec2(previewWidth, previewHeight));

		// Remove padding as it is only a problem for this panel.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));

		ImGui::Begin("Source Preview", nullptr,
		             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
		                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
		ImGui::Image(image.texture, ImVec2(previewWidth, previewHeight));
		ImGui::End();

		ImGui::PopStyleVar();
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center);

	if (config->aboutOpened) {
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5F, 0.5F));

		ImGui::Begin("About", &config->aboutOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("A lithophane generation tool with 3D preview and customisation.");
		ImGui::Spacing();
		ImGui::Text("Version: 1.0.0");
		ImGui::TextLinkOpenURL("Project GitHub", "https://github.com/yellowatom/lithogen");
		ImGui::End();
	}

	if (config->helpOpened) {
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5F, 0.5F));

		ImGui::Begin("Help", &config->helpOpened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		if (ImGui::CollapsingHeader("Importing and Exporting", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Under file, dialogues for loading images and saving models can be found.");
		}

		if (ImGui::CollapsingHeader("View Customisation", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text(
				"Under view, various checkboxes can be found to hide or adjust elements of the 3D model viewer.");
		}

		if (ImGui::CollapsingHeader("Settings Explanation", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SeparatorText("Mesh Type");
			ImGui::Text("Which shape the lithophane image will be placed on.");

			ImGui::SeparatorText("Dimensions");
			ImGui::Text("The width and height of the final model in millimeters.");

			ImGui::SeparatorText("Thickness");
			ImGui::Text(
				"The minium thickness will determine how much space is inbetween the back of the model and the\n"
				"lowest point in the lithophane depth, the maximum thickness will be the highest point the\n"
				"lithophane topology can reach.");

			ImGui::SeparatorText("Grayscale Preference");
			ImGui::Text("This setting adjusts how red, green and blue are weighted when generating the single height\n"
			            "value per pixel, this should usually be left as default unless there is a specific reason to\n"
			            "change it.");
		}

		ImGui::End();
	}
}