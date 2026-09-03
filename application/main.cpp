#include<Windows.h>
#include <cstdint>
#include <string>
#include<format>

#include <cassert>

#include "engine/core/Mymath.h"

#include "engine/core/Input.h"
#include "engine/core/WinApp.h"
#include "engine/core/DirectXCommon.h"
#include "engine/core/D3DResourceLeakChecker.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/3d/Object3dCommon.h"
#include "engine/3d/Object3d.h"
#include "engine/3d/Camera.h"
#include "engine/3d/SrvManager.h"
#include "engine/3d/TextureManager.h"

#pragma comment(lib,"dxcompiler.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Windowsアプリのエントリーポイント(main関数)
// Windowsアプリケーションのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;  // 終了時にD3Dリソースリークを確認する

#pragma region Windowの生成

	// ポインタ
	WinApp* winApp = nullptr;  // WindowsAPI管理クラス

	// WindowsAPIの初期化
	winApp = new WinApp();
	winApp->Initialize();

#pragma endregion

#pragma region 入力

	// 入力のポインタ
	Input* input = nullptr;  // 入力管理クラス

	// 入力の初期化
	input = new Input();
	input->Initialize(winApp);

#pragma endregion

#pragma region DirectXCommon

	DirectXCommon* dxCommon = nullptr;  // DirectX共通処理

	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	SrvManager* srvManager = new SrvManager();
	srvManager->Initialize(dxCommon);
	TextureManager* textureManager = new TextureManager();
	textureManager->Initialize(dxCommon, srvManager);

#pragma endregion

#pragma region Sprite系

	// SpriteCommon
	SpriteCommon* spriteCommon = nullptr;  // スプライト描画共通処理

	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	// Sprite
	Sprite* sprite = new Sprite();  // 表示するスプライト
	sprite->Initialize(spriteCommon);

#pragma endregion

	const uint32_t uvCheckerTexture = textureManager->Load("resource/uvChecker.png");
	const uint32_t monsterBallTexture = textureManager->Load("resource/monsterBall.png");
	const D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = textureManager->GetSrvHandleGPU(uvCheckerTexture);
	const D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = textureManager->GetSrvHandleGPU(monsterBallTexture);

	// Object3d共通部
	Object3dCommon* object3dCommon = new Object3dCommon();  // 3D描画共通処理
	object3dCommon->Initialize(dxCommon);

	// Object3d
	Object3d* object3d = new Object3d();  // OBJモデル表示用オブジェクト
	object3d->Initialize(object3dCommon, textureManager);
	Camera* camera = new Camera();
	camera->Update();

	uint32_t SphereVertexNum = 16 * 16 * 6;  // 球メッシュの頂点数

	//Sphere
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = dxCommon->CreateBufferResource(sizeof(VertexData) * SphereVertexNum);  // 球の頂点バッファ

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * SphereVertexNum;
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResourceSphere = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

	TransformationMatrix* wvpDateSphere = nullptr;
	wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDateSphere));
	wvpDateSphere->World = MakeIdentity4x4();

	VertexData* vertexDataSphere = nullptr;
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));

	// 球体用マテリアル
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSphere = dxCommon->CreateBufferResource(sizeof(Material));

	Material* materialDateSphere = nullptr;                                                
	materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDateSphere));
	materialDateSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);                           
	materialDateSphere->enableLighting = true;
	materialDateSphere->uvTransform = MakeIdentity4x4();

	//球体マテリアルのライト用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightSphereResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightSphereData = nullptr;
	directionalLightSphereResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightSphereData));
	directionalLightSphereData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightSphereData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightSphereData->intensity = 1.0f;


	Material* materialDateSprite = sprite->GetMaterialData();
	TransformationMatrix* transformationMatrixDataSprite = sprite->GetTransformationMatrixData();

	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformSphere{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform transformL{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{
		{ 1.0f,1.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 0.0f,0.0f,0.0f }
	};

	float* inputMaterialSphere[3] = { &materialDateSphere->color.x,&materialDateSphere->color.y,&materialDateSphere->color.z };
	float* inputTransformSphere[3] = { &transformSphere.translate.x,&transformSphere.translate.y,&transformSphere.translate.z };
	float* inputRotateSphere[3] = { &transformSphere.rotate.x,&transformSphere.rotate.y,&transformSphere.rotate.z };
	float* inputScaleSphere[3] = { &transformSphere.scale.x,&transformSphere.scale.y,&transformSphere.scale.z };
	bool textureChange = true;

	float* inputMaterialLigth[3] = { &directionalLightSphereData->color.x,&directionalLightSphereData->color.y,&directionalLightSphereData->color.z };
	float* inputDirectionLight[3] = { &directionalLightSphereData->direction.x,&directionalLightSphereData->direction.y,&directionalLightSphereData->direction.z };
	float* intensity = &directionalLightSphereData->intensity;

	bool isRotate = false;
	bool isModel = false;
	bool isSphere = true;
	bool isSprite = false;

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		if (winApp->ProcessMessege()) {
			// ゲームループを抜ける
			break;
		}
		else {

			//==============================
			// DirectXの毎フレームの処理
			//==============================

			//// ImGui関連 ////

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// Model //
			ImGui::Text("Model");

			ImGui::Checkbox("Model", &isModel);
			ImGui::Checkbox("Sphere", &isSphere);

			ImGui::InputFloat3("MaterialSphere", *inputMaterialSphere);
			ImGui::SliderFloat3("SliderMaterialSphere", *inputMaterialSphere, 0.0f, 1.0f);

			ImGui::InputFloat3("VertexSphere", *inputTransformSphere);
			ImGui::SliderFloat3("SliderVertexSphere", *inputTransformSphere, -5.0f, 5.0f);

			ImGui::Checkbox("Rotate", &isRotate);
			ImGui::InputFloat3("RotateSphere", *inputRotateSphere);
			ImGui::SliderFloat3("SliderRotateSphere", *inputRotateSphere, -10.0f, 10.0f);

			ImGui::InputFloat3("ScaleSphere", *inputScaleSphere);
			ImGui::SliderFloat3("SliderScaleSphere", *inputScaleSphere, 0.5f, 5.0f);

			ImGui::Checkbox("MonsterBall", &textureChange);

			// Lightng //
			ImGui::Text("Ligth");
			ImGui::InputFloat4("MaterialLigth", *inputMaterialLigth);
			ImGui::SliderFloat4("SliderMaterialLigth", *inputMaterialLigth, 0.0f, 1.0f);

			ImGui::InputFloat3("VertexLigth", *inputDirectionLight);
			ImGui::SliderFloat3("SliderVertexLigth", *inputDirectionLight, -1.0f, 1.0f);

			ImGui::InputFloat("intensity", intensity);

			// Sprite //
			ImGui::Text("Sprite");

			ImGui::Checkbox("UI", &isSprite);

			ImGui::InputFloat("SpriteX", &transformSprite.translate.x);
			ImGui::SliderFloat("SliderSpriteX", &transformSprite.translate.x, 0.0f, 1000.0f);

			ImGui::InputFloat("SpriteY", &transformSprite.translate.y);
			ImGui::SliderFloat("SliderSpriteY", &transformSprite.translate.y, 0.0f, 600.0f);

			ImGui::DragFloat2("UVTranlate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
			ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);

			ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

			//===============
			//ゲームの処理
			//===============

			input->Update();

			if (input->PushKey(DIK_0)) {
				OutputDebugStringA("Hit 0\n");
			}

			if (isRotate) {
				transformSphere.rotate.y -= 0.05f;
			}

			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			camera->Update();
			const Matrix4x4& viewProjectionMatrix = camera->GetViewProjectionMatrix();
			Matrix4x4 WorldViewProjectionMatrix = Multiply(worldMatrix, viewProjectionMatrix);
			object3d->GetTransformationMatrixData()->World = worldMatrix;
			object3d->GetTransformationMatrixData()->WVP = WorldViewProjectionMatrix;
			object3d->GetDirectionalLightData()->direction = Normalize(object3d->GetDirectionalLightData()->direction);

			// 球体
			Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
			Matrix4x4 WorldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, viewProjectionMatrix);

			wvpDateSphere->World = worldMatrixSphere;
			wvpDateSphere->WVP = WorldViewProjectionMatrixSphere;

			DrawSphere(vertexDataSphere);

			directionalLightSphereData->direction = Normalize(directionalLightSphereData->direction);

			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, (float)WinApp::kClientWidth, (float)WinApp::kClientHeight, 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

			Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
			uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
			materialDateSprite->uvTransform = uvTransformMatrix;

			// ImGuiの内部コマンド
			ImGui::Render();

			dxCommon->PreDraw();
			srvManager->PreDraw();

			// Object3d共通部の描画設定
			object3dCommon->CommonDrawSetting();

			// 球体
			if (isSphere) {
				dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);
				dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]
				dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());

				if (textureChange == false) {
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
				}
				else {
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
				}

				dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightSphereResource->GetGPUVirtualAddress());
				dxCommon->GetCommandList()->DrawInstanced(SphereVertexNum, 1, 0, 0);
			}

			// model
			if (isModel) {
				object3d->Draw();
			}

			// UI
			if (isSprite) {
				spriteCommon->CommonDrawSetting();

				D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite = sprite->GetVertexBufferView();
				D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite = sprite->GetIndexBufferView();

				dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
				dxCommon->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite);

				dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(0, sprite->GetMaterialResource()->GetGPUVirtualAddress()); //rootParameterの配列の0番目 [0]

				dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, sprite->GetTransformationMatrixResource()->GetGPUVirtualAddress());

				if (textureChange == 0) {
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
				}
				else {
					dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
				}

				dxCommon->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
			}

			//実際のcommandListのImGui描画コマンドを挟む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

			dxCommon->PostDraw();
		}
	}

	// 入力解放
	delete input;

	// WindowsAPI解放
	delete winApp;

	// Sprite系解放
	delete spriteCommon;
	delete sprite;

	// Object3d系解放
	delete object3d;
	delete object3dCommon;
	delete camera;

	delete textureManager;
	delete srvManager;

	// DirectXCommon解放
	delete dxCommon;

	return 0;
}
