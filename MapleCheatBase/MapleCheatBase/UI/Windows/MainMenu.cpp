#include "MainMenu.h"

#include <filesystem>
#include <fstream>

#include "imgui.h"
#include "xorstr.hpp"

#include "../StyleProvider.h"
#include "../Widgets/Widgets.h"
#include "../../Storage/Storage.h"
#include "../Widgets/3rd-party/FileDialog/imfilebrowser.h"
#include "../../Storage/StorageConfig.h"
#include "../../Utilities/Clipboard/ClipboardUtilities.h"
#include "../../Logging/Logger.h"
bool backgroundImageDialogInitialized = false;
ImGui::FileBrowser backgroundImageDialog;

bool replayDialogInitialized = false;
ImGui::FileBrowser replayDialog;

void MainMenu::updateBackground()
{
	/*if (Config::Visuals::UI::MenuBackground[0] == '\0' || !std::filesystem::exists(Config::Visuals::UI::MenuBackground))
	{
		if (backgroundTexture != nullptr)
		{
			if (UI::Renderer == Renderer::OGL3)
				TextureLoader::FreeTextureOGL3(backgroundTexture);

			backgroundTexture = nullptr;
		}

		return;
	}

	if (UI::Renderer == Renderer::OGL3)
		backgroundTexture = TextureLoader::LoadTextureFromFileOGL3(Config::Visuals::UI::MenuBackground);
	else
		backgroundTexture = TextureLoader::LoadTextureFromFileD3D9(UI::D3D9Device, Config::Visuals::UI::MenuBackground);*/
}

void MainMenu::Render()
{
	if (!isVisible)
		return;

	const ImGuiIO& io = ImGui::GetIO();
	const ImGuiStyle& style = ImGui::GetStyle();

	if (backgroundTexture != nullptr)
		ImGui::GetBackgroundDrawList()->AddImage(backgroundTexture, ImVec2(0, 0), ImVec2(io.DisplaySize.x, io.DisplaySize.y));

	// TODO: Update position here!
	const Vector2 clientPos = Vector2(0, 0);
	const Vector2 clientSize = Vector2(1920, 1080);

	const bool expanded = currentTab != -1;
	ImGui::SetNextWindowSize(expanded ? StyleProvider::MainMenuSize : StyleProvider::MainMenuSideBarSize);
	ImGui::SetNextWindowPos(ImVec2(clientSize.X / 2 - StyleProvider::MainMenuSize.x / 2, clientSize.Y / 2 - StyleProvider::MainMenuSize.y / 2), ImGuiCond_Once);
	ImGui::Begin(xorstr_("Main Menu"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		const ImVec2 menuSize = ImGui::GetCurrentWindow()->Size;
		const ImVec2 menuPos = ImGui::GetCurrentWindow()->Pos;

		ImGui::GetWindowDrawList()->AddRectFilled(menuPos, menuPos + StyleProvider::MainMenuSideBarSize, ImColor(StyleProvider::MenuColourDark), style.WindowRounding, expanded ? ImDrawFlags_RoundCornersAll & ~ImDrawFlags_RoundCornersTopRight : ImDrawFlags_RoundCornersAll);

		ImGui::SetCursorPos(StyleProvider::Padding);
		ImGui::BeginChild(xorstr_("Side Bar"), StyleProvider::MainMenuSideBarSize - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground);
		{
			const ImVec2 sideBarSize = ImGui::GetCurrentWindow()->Size;

			ImGui::PushFont(StyleProvider::FontHugeBold);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5 * StyleProvider::Scale, 10 * StyleProvider::Scale));
			ImGui::SetCursorPosX(sideBarSize.x / 2 - ((ImGui::CalcTextSize(xorstr_("Maple")).x / 2) + StyleProvider::MapleLogoSize.x / 2 + style.ItemSpacing.x / 2));
			ImGui::Image(StyleProvider::MapleLogoTexture, StyleProvider::MapleLogoSize, ImVec2(0, 0), ImVec2(1, 1), StyleProvider::AccentColour);
			ImGui::SameLine();
			ImGui::TextColored(StyleProvider::AccentColour, xorstr_("Maple"));
			ImGui::PopStyleVar();
			ImGui::PopFont();
			ImGui::PushFont(StyleProvider::FontSmall);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
			ImGui::SetCursorPosX(sideBarSize.x / 2 - ImGui::CalcTextSize(xorstr_("the quickest way to the top")).x / 2);
			ImGui::TextColored(StyleProvider::MottoColour, xorstr_("the quickest way to the top"));
			ImGui::PopFont();

			ImGui::Spacing();

			ImGui::BeginChild(xorstr_("User Info"), ImVec2(sideBarSize.x, StyleProvider::MainMenuUserInfoHeight), false, ImGuiWindowFlags_NoBackground);
			{
				const ImVec2 userInfoPos = ImGui::GetCurrentWindow()->Pos;
				const ImVec2 userInfoSize = ImGui::GetCurrentWindow()->Size;

				ImGui::GetWindowDrawList()->AddRectFilled(userInfoPos, userInfoPos + userInfoSize, ImColor(StyleProvider::MenuColourVeryDark), style.WindowRounding);

				ImGui::GetWindowDrawList()->AddImageRounded(StyleProvider::AvatarTexture, userInfoPos + ImVec2(userInfoSize.y / 4, userInfoSize.y / 4), userInfoPos + ImVec2(userInfoSize.y / 4 + userInfoSize.y / 2, userInfoSize.y / 4 + userInfoSize.y / 2), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), style.FrameRounding);

				ImGui::PushFont(StyleProvider::FontDefaultBold);
				ImGui::SetCursorPos(ImVec2(userInfoSize.y / 4 + userInfoSize.y / 2 + style.ItemSpacing.x, userInfoSize.y / 2 - style.ItemSpacing.y / 4 - ImGui::CalcTextSize("Welcome back").y));
				ImGui::Text(xorstr_("Welcome back"));
				ImGui::PopFont();

				ImGui::PushFont(StyleProvider::FontDefault);
				ImGui::SetCursorPos(ImVec2(userInfoSize.y / 4 + userInfoSize.y / 2 + style.ItemSpacing.x, userInfoSize.y / 2 + style.ItemSpacing.y / 4));
				ImGui::TextColored(StyleProvider::AccentColour, Communication::GetUser()->GetUsername().c_str());
				ImGui::PopFont();
			}
			ImGui::EndChild();

			ImGui::BeginChild(xorstr_("Tabs"), ImVec2(sideBarSize.x, sideBarSize.y - ImGui::GetCursorPosY() - StyleProvider::MainMenuBuildInfoHeight - style.ItemSpacing.y), false, ImGuiWindowFlags_NoBackground);
			{
				const ImVec2 tabsPos = ImGui::GetCurrentWindow()->Pos;
				const ImVec2 tabsSize = ImGui::GetCurrentWindow()->Size;

				ImGui::GetWindowDrawList()->AddRectFilled(tabsPos, tabsPos + tabsSize, ImColor(StyleProvider::MenuColourVeryDark), style.WindowRounding);

				const float tabsHeight = (40 * StyleProvider::Scale) * 8; //scaled tab height * tab count
				ImGui::SetCursorPos(ImVec2(StyleProvider::Padding.x, tabsSize.y / 2 - tabsHeight / 2));
				ImGui::BeginChild(xorstr_("Tabs##001"), ImVec2(tabsSize.x - (StyleProvider::Padding.x * 2), tabsHeight), false, ImGuiWindowFlags_NoBackground);
				{
					const ImVec2 tabSize = ImVec2(ImGui::GetCurrentWindow()->Size.x, 40 * StyleProvider::Scale);

					ImGui::PushFont(StyleProvider::FontDefaultBold);
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

					if (Widgets::Tab(xorstr_("Example"), StyleProvider::RelaxIconTexture, currentTab == 0, ImGuiSelectableFlags_SpanAllColumns, tabSize))
						currentTab = currentTab == 0 ? -1 : 0;

					ImGui::PopStyleVar();
					ImGui::PopFont();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();

			ImGui::BeginChild(xorstr_("Build Info"), ImVec2(sideBarSize.x, StyleProvider::MainMenuBuildInfoHeight), false, ImGuiWindowFlags_NoBackground);
			{
				const ImVec2 buildInfoPos = ImGui::GetCurrentWindow()->Pos;
				const ImVec2 buildInfoSize = ImGui::GetCurrentWindow()->Size;

				ImGui::GetWindowDrawList()->AddRectFilled(buildInfoPos, buildInfoPos + buildInfoSize, ImColor(StyleProvider::MenuColourVeryDark), style.WindowRounding);

				ImGui::PushFont(StyleProvider::FontSmallBold);
				const ImVec2 cheatInfoSize = ImGui::CalcTextSize(xorstr_("Maple for UNKNOWN!"));
				ImGui::SetCursorPos(ImVec2(buildInfoSize.x / 2 - cheatInfoSize.x / 2, buildInfoSize.y / 2 - style.ItemSpacing.y / 4 - cheatInfoSize.y));
				ImGui::TextColored(StyleProvider::AccentColour, xorstr_("Maple for UNKNOWN!"));
				ImGui::PopFont();

				ImGui::PushFont(StyleProvider::FontSmall);
				const ImVec2 buildStringSize = ImGui::CalcTextSize(xorstr_("VERSION"));
				ImGui::SetCursorPos(ImVec2(buildInfoSize.x / 2 - buildStringSize.x / 2, buildInfoSize.y / 2 + style.ItemSpacing.y / 4));
				ImGui::TextColored(StyleProvider::MottoColour, xorstr_("VERSION"));
				ImGui::PopFont();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(StyleProvider::MainMenuSideBarSize.x, 0) + StyleProvider::Padding);
		ImGui::BeginChild(xorstr_("Options"), ImVec2(StyleProvider::MainMenuSize.x - StyleProvider::MainMenuSideBarSize.x, StyleProvider::MainMenuSize.y) - StyleProvider::Padding * 2, false, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::PushFont(StyleProvider::FontDefault);

			const float optionsWidth = ImGui::GetWindowWidth();

			if (currentTab == 0)
			{
				Widgets::BeginPanel(xorstr_("Example"), ImVec2(optionsWidth, Widgets::CalcPanelHeight(6)));
				{
					Widgets::Checkbox(xorstr_("Enabled"), nullptr); ImGui::SameLine();
					Widgets::Tooltip(xorstr_("Test"));
				}
				Widgets::EndPanel();
			}

			ImGui::PopFont();
		}
		ImGui::EndChild();

		ImGui::SetWindowPos(ImVec2(std::clamp(menuPos.x, clientPos.X, clientPos.X + clientSize.X - menuSize.x), std::clamp(menuPos.y, clientPos.Y, clientPos.Y + clientSize.Y - menuSize.y)));
	}

	ImGui::End();
}

void MainMenu::Show()
{
	isVisible = true;
}

void MainMenu::Hide()
{
	isVisible = false;
}

void MainMenu::ToggleVisibility()
{
	isVisible = !isVisible;
}

bool MainMenu::GetIsVisible()
{
	return isVisible;
}