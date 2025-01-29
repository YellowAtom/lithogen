#pragma once

struct MenuConfig {
	bool drawSource = true;
	bool drawPreview = true;
	bool exampleCheckbox = true;
};

void MenuInit();
void MenuDraw();
void MenuShutdown();
