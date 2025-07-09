#pragma once

#include "base/PipelineStateObject.h"
#include <list>
#include <memory>
class DirectXCommon;
class Collider;
class GameCharacter;
class CollisionManager {


public:

	////////////////////////////////////////////////////////////////////////////////////////
	///		publicメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////


	/// <summary>
	/// シングルトンインスタンスの取得関数
	/// </summary>
	/// <returns></returns>
	static CollisionManager* GetInstance();

	void Initialize(DirectXCommon* dxCommon);

	void PreDraw();

	/// <summary>
	/// シングルトンインスタンスの解放処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// コライダーリストへの登録をする関数
	/// </summary>
	void RegisterCollider(Collider* collider);

	/// <summary>
	/// コライダーリストをクリアする関数
	/// </summary>
	void ClearCollider();

	/// <summary>
	/// 全てのコライダーの衝突判定を行う関数
	/// </summary>
	void CheckAllCollisions();

	/// <summary>
	/// コライダー2つ衝突判定と応答処理
	/// </summary>
	/// <param name="colliderB">コライダーA</param>
	/// <param name="colliderB">コライダーB</param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);


	void RegisterGameCharacter(GameCharacter* gameCharacter);

	void ClearGameCharacter();

	void CheckAllCollisionsForGameCharacter();

	void CheckCollisionPairForGameCharacter(GameCharacter* gameCharacterA, GameCharacter* gameCharacterB);

private:

	//シングルトンインスタンス
	static CollisionManager* instance_;

	CollisionManager() = default;
	~CollisionManager() = default;
	CollisionManager(CollisionManager&) = delete;
	CollisionManager& operator=(CollisionManager&) = delete;

private:

	////////////////////////////////////////////////////////////////////////////////////////
	///		privateメンバ関数
	////////////////////////////////////////////////////////////////////////////////////////

	DirectXCommon* dxCommon_ = nullptr;

	//コライダーリスト
	std::list<Collider*> colliders_;

	std::list<GameCharacter*> gameCharacters_;

	std::unique_ptr<PSO> pso_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
};

