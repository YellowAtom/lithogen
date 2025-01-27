#pragma once

struct MenuConfig {
	bool drawPicture = true;
	bool renderMesh = true;
	bool exampleCheckbox = true;
};

void MenuInit();
void MenuDraw();
void MenuShutdown();
