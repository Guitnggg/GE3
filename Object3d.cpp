#include "Object3d.h"

#include "Object3dCommon.h"

void Object3d::Initialize(Object3dCommon* object3dCommon) {
	// 引数で受け取ったメンバ変数に記録する
	this->object3dCommon_ = object3dCommon;

	// objファイルの読み込み
	modelData = LoadObjectFile("resource", "plane.obj");
}