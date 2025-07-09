#include "CollisionManager.h"
#include "Collider.h"
#include "Vector3Math.h"
#include "DirectXCommon.h"
#include "application/Entity/GameCharacter.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"

CollisionManager* CollisionManager::instance_ = nullptr;

CollisionManager* CollisionManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CollisionManager();
	}
	return instance_;
}

//=============================================================================
// 初期化
//=============================================================================

void CollisionManager::Initialize(DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_WIREFRAME, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("CollisionPSO");
	rootSignature_ = pso_->GetGraphicRootSignature();
}

//=============================================================================
// 描画前処理
//=============================================================================

void CollisionManager::PreDraw() {
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
}

//=============================================================================
// シングルトンインスタンスの解放処理
//=============================================================================

void CollisionManager::Finalize() {
	pso_.reset();
	rootSignature_.Reset();
	delete instance_;
	instance_ = nullptr;
}

//=============================================================================
// コライダーリストへの登録をする関数
//=============================================================================

void CollisionManager::RegisterCollider(Collider* collider) {

	colliders_.push_back(collider);
}

//=============================================================================
// コライダーリストをクリアする関数
//=============================================================================

void CollisionManager::ClearCollider() {

	colliders_.clear();
}

//=============================================================================
// 全てのコライダーの衝突判定を行う関数
//=============================================================================

void CollisionManager::CheckAllCollisions() {

	//リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {

		//イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;

		//イテレータA+1からコライダーBを取得する(重複判定の回避)
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {

			//イテレータBからコライダーBを取得する
			Collider* colliderB = *itrB;

			//コライダーAとBの衝突判定
			CheckCollisionPair(colliderA, colliderB);
		}

	}

}

//=============================================================================
// コライダー2つ衝突判定と応答処理
//=============================================================================

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {

	//コライダーAとBの座標
	Vector3 posA, posB;

	//コライダーAとBの座標の取得
	posA = colliderA->GetWorldPos();
	posB = colliderB->GetWorldPos();

	//コライダーAとBの衝突判定
	if (Vector3Math::Length(posA - posB) <= colliderA->GetRadius() + colliderB->GetRadius()) {
		//コライダーAの衝突処理
		//colliderA->OnCollisionAction(colliderB);
		//コライダーBの衝突処理
		//colliderB->OnCollisionAction(colliderA);
	}
}

void CollisionManager::RegisterGameCharacter(GameCharacter* gameCharacter) {

	gameCharacters_.push_back(gameCharacter);
}

void CollisionManager::ClearGameCharacter() {

	gameCharacters_.clear();
}

void CollisionManager::CheckAllCollisionsForGameCharacter() {

	//リスト内のペアを総当たり
	std::list<GameCharacter*>::iterator itrA = gameCharacters_.begin();
	for (; itrA != gameCharacters_.end(); ++itrA) {
		//イテレータAからコライダーAを取得する
		GameCharacter* gameCharacterA = *itrA;
		//イテレータA+1からコライダーBを取得する(重複判定の回避)
		std::list<GameCharacter*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != gameCharacters_.end(); ++itrB) {
			//イテレータBからコライダーBを取得する
			GameCharacter* gameCharacterB = *itrB;
			//コライダーAとBの衝突判定
			CheckCollisionPairForGameCharacter(gameCharacterA, gameCharacterB);
		}
	}
}

void CollisionManager::CheckCollisionPairForGameCharacter(GameCharacter* gameCharacterA, GameCharacter* gameCharacterB) {

	//コライダーAとB
	Collider* colliderA = gameCharacterA->GetCollider();
	Collider* colliderB = gameCharacterB->GetCollider();

	//コライダーがnullptrだったらreturn
	if (!colliderA || !colliderB) return;

	//コライダーAとBの衝突判定
	if (colliderA->CheckCollision(colliderB)) {
		//コライダーAの衝突処理
		gameCharacterA->OnCollisionAction(gameCharacterB);
		//コライダーBの衝突処理
		gameCharacterB->OnCollisionAction(gameCharacterA);
	}
}
