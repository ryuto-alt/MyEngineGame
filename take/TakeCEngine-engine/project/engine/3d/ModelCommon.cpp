#include "ModelCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
ModelCommon* ModelCommon::instance_ = nullptr;

ModelCommon* ModelCommon::GetInstance() {
	if(instance_ == nullptr) {
		instance_ = new ModelCommon();
	}
	return instance_;
}

void ModelCommon::Initialize(DirectXCommon* dxCommon,SrvManager* srvManager) {
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
}

void ModelCommon::Finalize() {
	dxCommon_ = nullptr;
	delete instance_;
	instance_ = nullptr;
}
