#include <iostream>
#include "GL/glew.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include "offsets.h"
#include "vector3.h"
#include "defs.h"
#include <tchar.h>
#include <intrin.h>

struct State {
	uintptr_t keys[7];
};

typedef struct {
	uintptr_t actor_ptr;
	uintptr_t damage_handler_ptr;
	uintptr_t player_state_ptr;
	uintptr_t root_component_ptr;
	uintptr_t mesh_ptr;
	uintptr_t bone_array_ptr;
	int bone_count;
	bool is_visible;
} Enemy;

// Window / Process values
HWND valorant_window;
GLFWwindow* g_window;
int g_width;
int g_height;
int g_pid;
uintptr_t g_base_address;
ImU32 g_esp_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0.4F, 1));
ImU32 g_color_white = ImGui::ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 1));

// Cheat toggle values
bool g_overlay_visible{ false };
bool g_esp_enabled{ true };
bool g_esp_dormantcheck{ false };
bool g_headesp{ true };
bool g_boneesp{ true };
bool g_boxesp{ true };

// Pointers
uintptr_t g_local_player_controller;
uintptr_t g_local_player_pawn;
uintptr_t g_local_damage_handler;
uintptr_t g_camera_manager;
int g_local_team_id;

// Enemy list
std::vector<Enemy> enemy_collection{};

uintptr_t getBaseAddress(uintptr_t pid) {
	// Add your memory r/w code here
	return NULL;
}

template <typename T> T read(uintptr_t pid, uintptr_t address) {
	T t{};
	// Add your memory r/w code here
	return t;
}

template <typename T> void write(uintptr_t pid, uintptr_t address, T& buffer) {
	// Add your memory r/w code here
}

std::wstring s2ws(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

int retreiveValProcessId() {
	BYTE target_name[] = { 'V','A','L','O','R','A','N','T','-','W','i','n','6','4','-','S','h','i','p','p','i','n','g','.','e','x','e', 0 };
	std::wstring process_name = s2ws(std::string((char*)target_name));
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // 0 to get all processes
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof(entry);

	if (!Process32First(snapshot, &entry)) {
		return 0;
	}

	while (Process32Next(snapshot, &entry)) {
		if (std::wstring(entry.szExeFile) == process_name) {
			return entry.th32ProcessID;
		}
	}

	return 0;
}

static void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void setupWindow() {
	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit()) {
		std::cout << "glfwInit didnt work.\n";
		return;
	}

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (!monitor) {
		fprintf(stderr, "Failed to get primary monitor!\n");
		return;
	}

	g_width = glfwGetVideoMode(monitor)->width;
	g_height = glfwGetVideoMode(monitor)->height;

	glfwWindowHint(GLFW_FLOATING, true);
	glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_MAXIMIZED, true);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

	g_window = glfwCreateWindow(g_width, g_height, "Word", NULL, NULL);
	if (g_window == NULL) {
		std::cout << "Could not create window.\n";
		return;
	}

	glfwSetWindowAttrib(g_window, GLFW_DECORATED, false);
	glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, true);
	glfwSetWindowMonitor(g_window, NULL, 0, 0, g_width, g_height + 1, 0);

	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1); // Enable vsync

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(g_window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	ImFont* font = io.Fonts->AddFontFromFileTTF("Roboto-Light.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	IM_ASSERT(font != NULL);
}

void cleanupWindow() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window);
	glfwTerminate();
}

BOOL CALLBACK retreiveValorantWindow(HWND hwnd, LPARAM lparam) {
	DWORD process_id;
	GetWindowThreadProcessId(hwnd, &process_id);
	if (process_id == g_pid) {
		valorant_window = hwnd;
	}
	return TRUE;
}

void activateValorantWindow() {
	SetForegroundWindow(valorant_window);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void handleKeyPresses() {
	// Toggle overlay
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		g_overlay_visible = !g_overlay_visible;
		glfwSetWindowAttrib(g_window, GLFW_MOUSE_PASSTHROUGH, !g_overlay_visible);
		if (g_overlay_visible) {
			HWND overlay_window = glfwGetWin32Window(g_window);
			SetForegroundWindow(overlay_window);
		}
		else {
			activateValorantWindow();
		}
	}
}

uintptr_t decryptWorld(uintptr_t pid, uintptr_t base_address) {
	const auto key = read<uintptr_t>(pid, base_address + offsets::uworld_key);
	const auto state = read<State>(pid, base_address + offsets::uworld_state);
	const auto uworld_ptr = decrypt_uworld(key, (uintptr_t*)&state);
	return read<uintptr_t>(pid, uworld_ptr);
}


struct FText
{
	char _padding_[0x28];
	PWCHAR Name;
	DWORD Length;
};

std::vector<Enemy> retreiveValidEnemies(uintptr_t actor_array, int actor_count) {
	std::vector<Enemy> temp_enemy_collection{};
	size_t size = sizeof(uintptr_t);
	for (int i = 0; i < actor_count; i++) {
		uintptr_t actor = read<uintptr_t>(g_pid, actor_array + (i * size));
		if (actor == 0x00) {
			continue;
		}
		uintptr_t unique_id = read<uintptr_t>(g_pid, actor + offsets::unique_id);
		if (unique_id != 18743553) {
			continue;
		}
		uintptr_t mesh = read<uintptr_t>(g_pid, actor + offsets::mesh_component);
		if (!mesh) {
			continue;
		}

		uintptr_t player_state = read<uintptr_t>(g_pid, actor + offsets::player_state);
		uintptr_t team_component = read<uintptr_t>(g_pid, player_state + offsets::team_component);
		int team_id = read<int>(g_pid, team_component + offsets::team_id);
		int bone_count = read<int>(g_pid, mesh + offsets::bone_count);
		bool is_bot = bone_count == 103;
		if (team_id == g_local_team_id && !is_bot) {
			continue;
		}

		uintptr_t damage_handler = read<uintptr_t>(g_pid, actor + offsets::damage_handler);
		uintptr_t root_component = read<uintptr_t>(g_pid, actor + offsets::root_component);
		uintptr_t bone_array = read<uintptr_t>(g_pid, mesh + offsets::bone_array);

		Enemy enemy{
			actor,
			damage_handler,
			player_state,
			root_component,
			mesh,
			bone_array,
			bone_count,
			true
		};

		temp_enemy_collection.push_back(enemy);
	}

	return temp_enemy_collection;
}

void retreiveData() {
	while (true) {
		uintptr_t world = decryptWorld(g_pid, g_base_address);

		uintptr_t game_instance = read<uintptr_t>(g_pid, world + offsets::game_instance);
		uintptr_t persistent_level = read<uintptr_t>(g_pid, world + offsets::persistent_level);

		uintptr_t local_player_array = read<uintptr_t>(g_pid, game_instance + offsets::local_player_array);
		uintptr_t local_player = read<uintptr_t>(g_pid, local_player_array);
		uintptr_t local_player_controller = read<uintptr_t>(g_pid, local_player + offsets::local_player_controller);
		uintptr_t local_player_pawn = read<uintptr_t>(g_pid, local_player_controller + offsets::local_player_pawn);
		uintptr_t local_damage_handler = read<uintptr_t>(g_pid, local_player_pawn + offsets::damage_handler);
		uintptr_t local_player_state = read<uintptr_t>(g_pid, local_player_pawn + offsets::player_state);
		uintptr_t local_team_component = read<uintptr_t>(g_pid, local_player_state + offsets::team_component);
		int local_team_id = read<int>(g_pid, local_team_component + offsets::team_id);

		uintptr_t camera_manager = read<uintptr_t>(g_pid, local_player_controller + offsets::camera_manager);

		uintptr_t actor_array = read<uintptr_t>(g_pid, persistent_level + offsets::actor_array);
		int actor_count = read<int>(g_pid, persistent_level + offsets::actor_count);

		g_local_player_controller = local_player_controller;
		g_local_player_pawn = local_player_pawn;
		g_local_damage_handler = local_damage_handler;
		g_camera_manager = camera_manager;
		g_local_team_id = local_team_id;

		enemy_collection = retreiveValidEnemies(actor_array, actor_count);
		Sleep(2500);
	}
}

Vector3 getBonePosition(Enemy enemy, int index) {
	size_t size = sizeof(FTransform);
	FTransform firstBone = read<FTransform>(g_pid, enemy.bone_array_ptr + (size * index));
	FTransform componentToWorld = read<FTransform>(g_pid, enemy.mesh_ptr + offsets::component_to_world);
	D3DMATRIX matrix = MatrixMultiplication(firstBone.ToMatrixWithScale(), componentToWorld.ToMatrixWithScale());
	return Vector3(matrix._41, matrix._42, matrix._43);
}

void renderBoneLine(Vector3 first_bone_position, Vector3 second_bone_position, Vector3 position, Vector3 rotation, float fov) {
	Vector2 first_bone_screen_position = worldToScreen(first_bone_position, position, rotation, fov);
	ImVec2 fist_screen_position = ImVec2(first_bone_screen_position.x, first_bone_screen_position.y);
	Vector2 second_bone_screen_position = worldToScreen(second_bone_position, position, rotation, fov);
	ImVec2 second_screen_position = ImVec2(second_bone_screen_position.x, second_bone_screen_position.y);
	ImGui::GetOverlayDrawList()->AddLine(fist_screen_position, second_screen_position, g_color_white);
}

void renderBones(Enemy enemy, Vector3 position, Vector3 rotation, float fov) {
	Vector3 head_position = getBonePosition(enemy, 8);
	Vector3 neck_position;
	Vector3 chest_position = getBonePosition(enemy, 6);
	Vector3 l_upper_arm_position;
	Vector3 l_fore_arm_position;
	Vector3 l_hand_position;
	Vector3 r_upper_arm_position;
	Vector3 r_fore_arm_position;
	Vector3 r_hand_position;
	Vector3 stomach_position = getBonePosition(enemy, 4);
	Vector3 pelvis_position = getBonePosition(enemy, 3);
	Vector3 l_thigh_position;
	Vector3 l_knee_position;
	Vector3 l_foot_position;
	Vector3 r_thigh_position;
	Vector3 r_knee_position;
	Vector3 r_foot_position;
	if (enemy.bone_count == 102) { // MALE
		neck_position = getBonePosition(enemy, 19);

		l_upper_arm_position = getBonePosition(enemy, 21);
		l_fore_arm_position = getBonePosition(enemy, 22);
		l_hand_position = getBonePosition(enemy, 23);

		r_upper_arm_position = getBonePosition(enemy, 47);
		r_fore_arm_position = getBonePosition(enemy, 48);
		r_hand_position = getBonePosition(enemy, 49);

		l_thigh_position = getBonePosition(enemy, 75);
		l_knee_position = getBonePosition(enemy, 76);
		l_foot_position = getBonePosition(enemy, 78);

		r_thigh_position = getBonePosition(enemy, 82);
		r_knee_position = getBonePosition(enemy, 83);
		r_foot_position = getBonePosition(enemy, 85);
	}
	else if (enemy.bone_count == 99) { // FEMALE
		neck_position = getBonePosition(enemy, 19);

		l_upper_arm_position = getBonePosition(enemy, 21);
		l_fore_arm_position = getBonePosition(enemy, 40);
		l_hand_position = getBonePosition(enemy, 42);

		r_upper_arm_position = getBonePosition(enemy, 46);
		r_fore_arm_position = getBonePosition(enemy, 65);
		r_hand_position = getBonePosition(enemy, 67);

		l_thigh_position = getBonePosition(enemy, 78);
		l_knee_position = getBonePosition(enemy, 75);
		l_foot_position = getBonePosition(enemy, 77);

		r_thigh_position = getBonePosition(enemy, 80);
		r_knee_position = getBonePosition(enemy, 82);
		r_foot_position = getBonePosition(enemy, 84);
	}
	else if (enemy.bone_count == 103) { // BOT
		neck_position = getBonePosition(enemy, 9);

		l_upper_arm_position = getBonePosition(enemy, 33);
		l_fore_arm_position = getBonePosition(enemy, 30);
		l_hand_position = getBonePosition(enemy, 32);

		r_upper_arm_position = getBonePosition(enemy, 58);
		r_fore_arm_position = getBonePosition(enemy, 55);
		r_hand_position = getBonePosition(enemy, 57);

		l_thigh_position = getBonePosition(enemy, 63);
		l_knee_position = getBonePosition(enemy, 65);
		l_foot_position = getBonePosition(enemy, 69);

		r_thigh_position = getBonePosition(enemy, 77);
		r_knee_position = getBonePosition(enemy, 79);
		r_foot_position = getBonePosition(enemy, 83);
	}
	else {
		return;
	}

	renderBoneLine(head_position, neck_position, position, rotation, fov);
	renderBoneLine(neck_position, chest_position, position, rotation, fov);
	renderBoneLine(neck_position, l_upper_arm_position, position, rotation, fov);
	renderBoneLine(l_upper_arm_position, l_fore_arm_position, position, rotation, fov);
	renderBoneLine(l_fore_arm_position, l_hand_position, position, rotation, fov);
	renderBoneLine(neck_position, r_upper_arm_position, position, rotation, fov);
	renderBoneLine(r_upper_arm_position, r_fore_arm_position, position, rotation, fov);
	renderBoneLine(r_fore_arm_position, r_hand_position, position, rotation, fov);
	renderBoneLine(chest_position, stomach_position, position, rotation, fov);
	renderBoneLine(stomach_position, pelvis_position, position, rotation, fov);
	renderBoneLine(pelvis_position, l_thigh_position, position, rotation, fov);
	renderBoneLine(l_thigh_position, l_knee_position, position, rotation, fov);
	renderBoneLine(l_knee_position, l_foot_position, position, rotation, fov);
	renderBoneLine(pelvis_position, r_thigh_position, position, rotation, fov);
	renderBoneLine(r_thigh_position, r_knee_position, position, rotation, fov);
	renderBoneLine(r_knee_position, r_foot_position, position, rotation, fov);
}

void renderBox(Vector2 head_at_screen, float distance_modifier) {
	int head_x = head_at_screen.x;
	int head_y = head_at_screen.y;
	int start_x = head_x - 35 / distance_modifier;
	int start_y = head_y - 15 / distance_modifier;
	int end_x = head_x + 35 / distance_modifier;
	int end_y = head_y + 155 / distance_modifier;
	ImGui::GetOverlayDrawList()->AddRect(ImVec2(start_x, start_y), ImVec2(end_x, end_y), g_esp_color);
}

void renderEsp() {
	std::vector<Enemy> local_enemy_collection = enemy_collection;
	if (local_enemy_collection.empty()) {
		return;
	}

	Vector3 camera_position = read<Vector3>(g_pid, g_camera_manager + offsets::camera_position);
	Vector3 camera_rotation = read<Vector3>(g_pid, g_camera_manager + offsets::camera_rotation);
	float camera_fov = read<float>(g_pid, g_camera_manager + offsets::camera_fov);

	for (int i = 0; i < local_enemy_collection.size(); i++) {
		Enemy enemy = local_enemy_collection[i];
		float health = read<float>(g_pid, enemy.damage_handler_ptr + offsets::health);
		if (enemy.actor_ptr == g_local_player_pawn || health <= 0 || !enemy.mesh_ptr) {
			continue;
		}

		Vector3 head_position = getBonePosition(enemy, 8); // 8 = head bone
		Vector3 root_position = read<Vector3>(g_pid, enemy.root_component_ptr + offsets::root_position);
		if (head_position.z <= root_position.z) {
			continue;
		}

		if (g_esp_dormantcheck) {
			float last_render_time = read<float>(g_pid, enemy.mesh_ptr + offsets::last_render_time);
			float last_submit_time = read<float>(g_pid, enemy.mesh_ptr + offsets::last_submit_time);
			bool is_visible = last_render_time + 0.06F >= last_submit_time;
			bool dormant = read<bool>(g_pid, enemy.actor_ptr + offsets::dormant);
			if (!dormant || !is_visible) {
				continue;
			}
		}

		Vector2 head_at_screen_vec = worldToScreen(head_position, camera_position, camera_rotation, camera_fov);
		ImVec2 head_at_screen = ImVec2(head_at_screen_vec.x, head_at_screen_vec.y);
		float distance_modifier = camera_position.Distance(head_position) * 0.001F;

		if (g_boneesp) {
			renderBones(enemy, camera_position, camera_rotation, camera_fov);
		}
		if (g_headesp) {
			ImGui::GetOverlayDrawList()->AddCircle(head_at_screen, 7 / distance_modifier, g_esp_color, 0, 3);
		}
		if (g_boxesp) {
			renderBox(head_at_screen_vec, distance_modifier);
		}
	}
}

void runRenderTick() {
	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (g_esp_enabled) {
		renderEsp();
	}

	if (g_overlay_visible) {
		// Visuals Window
		{
			ImGui::Begin("Visuals", nullptr, ImGuiWindowFlags_NoResize);
			ImGui::SetWindowSize("Visuals", ImVec2(200, 176));

			ImGui::Checkbox("ESP", &g_esp_enabled);
			ImGui::Checkbox("ESP Dormant Check", &g_esp_dormantcheck);
			ImGui::Checkbox("Head ESP", &g_headesp);
			ImGui::Checkbox("Bone ESP", &g_boneesp);
			ImGui::Checkbox("Box ESP", &g_boxesp);
			ImGui::End();
		}
	}

	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(g_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(g_window);
}

int main()
{
	// Search for the process and save the process id
	g_pid = retreiveValProcessId();
	if (!g_pid) {
		std::cout << "Could not find val process id.\n";
		system("pause");
		return 1;
	}

	// Get the valorant game window
	EnumWindows(retreiveValorantWindow, NULL);
	if (!valorant_window) {
		std::cout << "Could not find val window.\n";
		system("pause");
		return 1;
	}

	// Get process base address
	g_base_address = getBaseAddress(g_pid);
	if (!g_base_address) {
		std::cout << "Could not get base address.\n";
		system("pause");
		return 1;
	}

	// Create the opengl overlay window and setup imgui
	setupWindow();
	if (!g_window) {
		std::cout << "Could not setup window.\n";
		system("pause");
		return 1;
	}

	// Retreive data loop thread
	HANDLE handle = CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)retreiveData, nullptr, NULL, nullptr);
	if (handle) {
		CloseHandle(handle);
	}

	// Main loop
	while (!glfwWindowShouldClose(g_window))
	{
		handleKeyPresses();
		runRenderTick();
	}

	// Cleanup
	cleanupWindow();
}
