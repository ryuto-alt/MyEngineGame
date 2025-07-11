#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

// GLB ファイルフォーマット定義
// 参考: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#glb-file-format-specification

namespace GLB {
    // GLBファイルヘッダー
    struct Header {
        uint32_t magic;        // 0x46546C67 ("glTF")
        uint32_t version;      // GLBバージョン（通常は2）
        uint32_t length;       // 総ファイルサイズ
    };

    // GLBチャンクヘッダー
    struct ChunkHeader {
        uint32_t length;       // チャンクデータのサイズ
        uint32_t type;         // チャンクタイプ
    };

    // GLBチャンクタイプ定数
    static const uint32_t CHUNK_TYPE_JSON = 0x4E4F534A;  // "JSON"
    static const uint32_t CHUNK_TYPE_BIN = 0x004E4942;   // "BIN\0"

    // GLBチャンクデータ
    struct Chunk {
        ChunkHeader header;
        std::vector<uint8_t> data;
    };

    // GLBファイル全体のデータ構造
    struct File {
        Header header;
        std::vector<Chunk> chunks;
        
        // JSON チャンクの取得
        const Chunk* GetJsonChunk() const;
        
        // バイナリチャンクの取得
        const Chunk* GetBinaryChunk() const;
    };

    // GLBファイル読み込みクラス
    class Parser {
    public:
        Parser();
        ~Parser();
        
        // GLBファイルを読み込む
        bool LoadFromFile(const std::string& filePath);
        
        // メモリからGLBファイルを読み込む
        bool LoadFromMemory(const uint8_t* data, size_t size);
        
        // 解析結果の取得
        const File& GetFile() const { return file_; }
        
        // エラーメッセージの取得
        const std::string& GetErrorMessage() const { return errorMessage_; }
        
    private:
        File file_;
        std::string errorMessage_;
        
        // バイナリデータの読み込み
        bool ReadBinaryData(const uint8_t* data, size_t size);
        
        // ヘッダーの読み込み
        bool ReadHeader(const uint8_t* data, size_t offset, size_t size);
        
        // チャンクの読み込み
        bool ReadChunk(const uint8_t* data, size_t offset, size_t size, Chunk& chunk);
        
        // リトルエンディアン形式でuint32_tを読み込む
        uint32_t ReadUint32LE(const uint8_t* data, size_t offset) const;
        
        // データの妥当性チェック
        bool ValidateHeader(const Header& header);
        bool ValidateChunk(const Chunk& chunk);
        
        // 4バイト境界へのパディング計算
        size_t CalculatePadding(size_t size) const;
    };
}