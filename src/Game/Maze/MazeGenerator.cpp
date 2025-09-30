#include "MazeGenerator.h"
#include <algorithm>
#include <queue>
#include <ctime>

MazeGenerator::MazeGenerator() {
    rng_.seed(static_cast<unsigned int>(std::time(nullptr)));
}

void MazeGenerator::Generate(MazeGrid* grid, const GenerationParams& params) {
    if (!grid) return;
    
    // シード設定
    if (params.seed != 0) {
        rng_.seed(params.seed);
    } else {
        rng_.seed(static_cast<unsigned int>(std::time(nullptr)));
    }
    
    // グリッドをリセット
    grid->Reset();
    
    // 初期状態：全てを壁にする
    grid->ForEachCell([](int x, int y, MazeCell* cell) {
        cell->SetType(MazeCellType::Wall);
        cell->SetVisited(false);
    });
    
    // アルゴリズムに応じて迷路を生成
    switch (params.algorithm) {
        case MazeAlgorithm::RecursiveBacktracking:
            GenerateRecursiveBacktracking(grid);
            break;
        case MazeAlgorithm::DarkDeceptionStyle:
            GenerateDarkDeceptionStyle(grid);
            break;
        default:
            GenerateRecursiveBacktracking(grid);
            break;
    }
    
    // ループを追加
    if (params.createLoops) {
        CreateLoops(grid, params.loopProbability);
    }
    
    // 部屋を追加
    if (params.addRooms) {
        GenerateRooms(grid, params.roomCount, params.minRoomSize, params.maxRoomSize);
    }
    
    // スタートとゴールを設定
    grid->SetStartPosition(0, 0);
    grid->SetGoalPosition(grid->GetWidth() - 1, grid->GetHeight() - 1);
    
    // Dark Deceptionエフェクトを追加
    AddDarkDeceptionEffects(grid);
}

void MazeGenerator::GenerateRecursiveBacktracking(MazeGrid* grid) {
    std::stack<std::pair<int, int>> stack;
    
    // 開始地点
    int startX = 0;
    int startY = 0;
    
    // 開始地点を通路にする
    MazeCell* startCell = grid->GetCell(startX, startY);
    if (startCell) {
        startCell->SetType(MazeCellType::Floor);
        startCell->SetVisited(true);
    }
    
    stack.push({startX, startY});
    
    while (!stack.empty()) {
        int currentX = stack.top().first;
        int currentY = stack.top().second;
        
        // 未訪問の隣接セルを探す
        std::vector<std::pair<Direction, std::pair<int, int>>> unvisitedNeighbors;
        
        for (int i = 0; i < 4; ++i) {
            Direction dir = static_cast<Direction>(i);
            int dx = 0, dy = 0;
            
            switch (dir) {
                case Direction::North: dy = -2; break;
                case Direction::East: dx = 2; break;
                case Direction::South: dy = 2; break;
                case Direction::West: dx = -2; break;
            }
            
            int nx = currentX + dx;
            int ny = currentY + dy;
            
            if (grid->IsValidPosition(nx, ny)) {
                MazeCell* neighbor = grid->GetCell(nx, ny);
                if (neighbor && !neighbor->IsVisited()) {
                    unvisitedNeighbors.push_back({dir, {nx, ny}});
                }
            }
        }
        
        if (!unvisitedNeighbors.empty()) {
            // ランダムに隣接セルを選択
            std::uniform_int_distribution<int> dist(0, static_cast<int>(unvisitedNeighbors.size() - 1));
            int index = dist(rng_);
            
            auto [dir, nextPos] = unvisitedNeighbors[index];
            int nextX = nextPos.first;
            int nextY = nextPos.second;
            
            // 壁を削除して通路を作る
            int wallX = currentX + (nextX - currentX) / 2;
            int wallY = currentY + (nextY - currentY) / 2;
            
            MazeCell* wallCell = grid->GetCell(wallX, wallY);
            if (wallCell) {
                wallCell->SetType(MazeCellType::Floor);
                wallCell->SetVisited(true);
            }
            
            MazeCell* nextCell = grid->GetCell(nextX, nextY);
            if (nextCell) {
                nextCell->SetType(MazeCellType::Floor);
                nextCell->SetVisited(true);
            }
            
            // 接続を設定
            MazeCell* currentCell = grid->GetCell(currentX, currentY);
            if (currentCell && nextCell) {
                currentCell->SetConnected(dir, true);
                Direction oppositeDir = (dir == Direction::North) ? Direction::South :
                                      (dir == Direction::East) ? Direction::West :
                                      (dir == Direction::South) ? Direction::North :
                                      Direction::East;
                nextCell->SetConnected(oppositeDir, true);
            }
            
            stack.push({nextX, nextY});
        } else {
            stack.pop();
        }
    }
}

void MazeGenerator::GenerateDarkDeceptionStyle(MazeGrid* grid) {
    // Dark Deception風: 長い通路と少ない分岐を特徴とする
    
    // メインパスを作成
    CreateMainPath(grid);
    
    // 分岐を追加
    AddBranches(grid, 0.3f);
    
    // いくつかの大きな部屋を追加
    GenerateRooms(grid, 4, 4, 6);
    
    // ループを作成してプレイヤーが追いかけられても逃げられるようにする
    CreateLoops(grid, 0.15f);
}

void MazeGenerator::CreateMainPath(MazeGrid* grid) {
    int width = grid->GetWidth();
    int height = grid->GetHeight();
    
    // スネーク状のメインパスを作成
    int x = 0, y = 0;
    
    while (x < width - 1 || y < height - 1) {
        MazeCell* cell = grid->GetCell(x, y);
        if (cell) {
            cell->SetType(MazeCellType::Floor);
            cell->SetVisited(true);
        }
        
        // 進行方向を決定
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        float chance = dist(rng_);
        
        if (x >= width - 1) {
            // 右端に到達したら下へ
            if (y < height - 1) y++;
        } else if (y >= height - 1) {
            // 下端に到達したら右へ
            if (x < width - 1) x++;
        } else {
            // ランダムに右か下へ（右を優先）
            if (chance < 0.7f && x < width - 1) {
                x++;
            } else if (y < height - 1) {
                y++;
            } else if (x < width - 1) {
                x++;
            }
        }
    }
    
    // ゴール地点も通路にする
    MazeCell* goalCell = grid->GetCell(width - 1, height - 1);
    if (goalCell) {
        goalCell->SetType(MazeCellType::Floor);
        goalCell->SetVisited(true);
    }
}

void MazeGenerator::AddBranches(MazeGrid* grid, float branchingFactor) {
    int width = grid->GetWidth();
    int height = grid->GetHeight();
    
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> lengthDist(3, 8);
    
    // 既存の通路から分岐を作成
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x += 2) {
            MazeCell* cell = grid->GetCell(x, y);
            
            if (cell && cell->GetType() == MazeCellType::Floor) {
                if (chanceDist(rng_) < branchingFactor) {
                    // ランダムな方向に分岐を作成
                    auto directions = GetShuffledDirections();
                    
                    for (Direction dir : directions) {
                        int length = lengthDist(rng_);
                        CreateLongCorridor(grid, x, y, dir, length);
                        break; // 1方向のみ
                    }
                }
            }
        }
    }
}

void MazeGenerator::CreateLongCorridor(MazeGrid* grid, int startX, int startY, Direction dir, int length) {
    int dx = 0, dy = 0;
    
    switch (dir) {
        case Direction::North: dy = -1; break;
        case Direction::East: dx = 1; break;
        case Direction::South: dy = 1; break;
        case Direction::West: dx = -1; break;
    }
    
    int x = startX;
    int y = startY;
    
    for (int i = 0; i < length; ++i) {
        x += dx;
        y += dy;
        
        if (!grid->IsValidPosition(x, y)) {
            break;
        }
        
        MazeCell* cell = grid->GetCell(x, y);
        if (cell && cell->GetType() == MazeCellType::Wall) {
            cell->SetType(MazeCellType::Floor);
            cell->SetVisited(true);
        }
    }
}

void MazeGenerator::CreateRoom(MazeGrid* grid, int centerX, int centerY, int width, int height) {
    int startX = centerX - width / 2;
    int startY = centerY - height / 2;
    int endX = centerX + width / 2;
    int endY = centerY + height / 2;
    
    // 範囲をグリッド内に制限
    startX = (std::max)(0, startX);
    startY = (std::max)(0, startY);
    endX = (std::min)(grid->GetWidth() - 1, endX);
    endY = (std::min)(grid->GetHeight() - 1, endY);
    
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            MazeCell* cell = grid->GetCell(x, y);
            if (cell) {
                cell->SetType(MazeCellType::Floor);
                cell->SetVisited(true);
            }
        }
    }
}

void MazeGenerator::CreateLoops(MazeGrid* grid, float probability) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (int y = 1; y < grid->GetHeight() - 1; ++y) {
        for (int x = 1; x < grid->GetWidth() - 1; ++x) {
            MazeCell* cell = grid->GetCell(x, y);
            
            if (cell && cell->GetType() == MazeCellType::Wall) {
                // 周囲の通路の数を数える
                int pathCount = 0;
                
                for (int i = 0; i < 4; ++i) {
                    Direction dir = static_cast<Direction>(i);
                    MazeCell* neighbor = grid->GetNeighbor(x, y, dir);
                    if (neighbor && neighbor->GetType() == MazeCellType::Floor) {
                        pathCount++;
                    }
                }
                
                // 2つ以上の通路に接している壁をループ作成候補とする
                if (pathCount >= 2 && dist(rng_) < probability) {
                    cell->SetType(MazeCellType::Floor);
                }
            }
        }
    }
}

void MazeGenerator::GenerateRooms(MazeGrid* grid, int roomCount, int minSize, int maxSize) {
    std::uniform_int_distribution<int> sizeDist(minSize, maxSize);
    std::uniform_int_distribution<int> xDist(2, grid->GetWidth() - 3);
    std::uniform_int_distribution<int> yDist(2, grid->GetHeight() - 3);
    
    for (int i = 0; i < roomCount; ++i) {
        int roomWidth = sizeDist(rng_);
        int roomHeight = sizeDist(rng_);
        
        // ランダムな位置を試す
        for (int attempt = 0; attempt < 10; ++attempt) {
            int centerX = xDist(rng_);
            int centerY = yDist(rng_);
            
            if (CanPlaceRoom(grid, centerX, centerY, roomWidth, roomHeight)) {
                CreateRoom(grid, centerX, centerY, roomWidth, roomHeight);
                break;
            }
        }
    }
}

bool MazeGenerator::CanPlaceRoom(MazeGrid* grid, int x, int y, int width, int height) {
    int startX = x - width / 2 - 1;
    int startY = y - height / 2 - 1;
    int endX = x + width / 2 + 1;
    int endY = y + height / 2 + 1;
    
    // 範囲がグリッド内に収まるか確認
    if (startX < 0 || startY < 0 || endX >= grid->GetWidth() || endY >= grid->GetHeight()) {
        return false;
    }
    
    // 既存の部屋と重ならないか確認
    int floorCount = 0;
    for (int py = startY; py <= endY; ++py) {
        for (int px = startX; px <= endX; ++px) {
            MazeCell* cell = grid->GetCell(px, py);
            if (cell && cell->GetType() == MazeCellType::Floor) {
                floorCount++;
                if (floorCount > 2) { // 少しの重なりは許可
                    return false;
                }
            }
        }
    }
    
    return true;
}

void MazeGenerator::AddDarkDeceptionEffects(MazeGrid* grid) {
    std::uniform_real_distribution<float> darknessDist(0.3f, 0.8f);
    std::uniform_real_distribution<float> fogDist(0.2f, 0.6f);
    std::uniform_int_distribution<int> zoneSizeDist(3, 6);
    
    int width = grid->GetWidth();
    int height = grid->GetHeight();
    
    // ランダムにダークゾーンを配置
    for (int i = 0; i < 5; ++i) {
        int zoneWidth = zoneSizeDist(rng_);
        int zoneHeight = zoneSizeDist(rng_);
        int x = std::uniform_int_distribution<int>(0, width - zoneWidth)(rng_);
        int y = std::uniform_int_distribution<int>(0, height - zoneHeight)(rng_);
        
        float darkness = darknessDist(rng_);
        grid->SetDarkZone(x, y, x + zoneWidth, y + zoneHeight, darkness);
    }
    
    // ランダムにフォグゾーンを配置
    for (int i = 0; i < 3; ++i) {
        int zoneWidth = zoneSizeDist(rng_);
        int zoneHeight = zoneSizeDist(rng_);
        int x = std::uniform_int_distribution<int>(0, width - zoneWidth)(rng_);
        int y = std::uniform_int_distribution<int>(0, height - zoneHeight)(rng_);
        
        float fogDensity = fogDist(rng_);
        grid->SetFoggyZone(x, y, x + zoneWidth, y + zoneHeight, fogDensity);
    }
}

void MazeGenerator::PlaceItems(MazeGrid* grid, int itemCount) {
    std::vector<std::pair<int, int>> floorCells;
    
    // 全ての通路セルを収集
    grid->ForEachCell([&floorCells](int x, int y, MazeCell* cell) {
        if (cell && cell->GetType() == MazeCellType::Floor) {
            floorCells.push_back({x, y});
        }
    });
    
    // ランダムにアイテムを配置
    std::shuffle(floorCells.begin(), floorCells.end(), rng_);
    
    for (int i = 0; i < itemCount && i < static_cast<int>(floorCells.size()); ++i) {
        MazeCell* cell = grid->GetCell(floorCells[i].first, floorCells[i].second);
        if (cell) {
            cell->SetType(MazeCellType::Item);
        }
    }
}

void MazeGenerator::PlaceEnemies(MazeGrid* grid, int enemyCount) {
    std::vector<std::pair<int, int>> floorCells;
    
    // 全ての通路セルを収集（スタート地点から離れた場所を優先）
    grid->ForEachCell([&floorCells, grid](int x, int y, MazeCell* cell) {
        if (cell && cell->GetType() == MazeCellType::Floor) {
            // スタート地点から一定距離離れていることを確認
            Vector2 startPos = grid->GetStartPosition();
            float dist = std::sqrt(std::pow(x - startPos.x, 2) + std::pow(y - startPos.y, 2));
            if (dist > 5.0f) {
                floorCells.push_back({x, y});
            }
        }
    });
    
    // ランダムに敵を配置
    std::shuffle(floorCells.begin(), floorCells.end(), rng_);
    
    for (int i = 0; i < enemyCount && i < static_cast<int>(floorCells.size()); ++i) {
        MazeCell* cell = grid->GetCell(floorCells[i].first, floorCells[i].second);
        if (cell) {
            cell->SetType(MazeCellType::Enemy);
        }
    }
}

std::vector<Vector2> MazeGenerator::FindDeadEnds(MazeGrid* grid) {
    std::vector<Vector2> deadEnds;
    
    grid->ForEachCell([&deadEnds, grid, this](int x, int y, MazeCell* cell) {
        if (cell && cell->GetType() == MazeCellType::Floor) {
            if (IsDeadEnd(grid, x, y)) {
                deadEnds.push_back(Vector2(static_cast<float>(x), static_cast<float>(y)));
            }
        }
    });
    
    return deadEnds;
}

bool MazeGenerator::IsDeadEnd(MazeGrid* grid, int x, int y) {
    int wallCount = CountWalls(grid, x, y);
    return wallCount >= 3; // 3方向が壁なら行き止まり
}

int MazeGenerator::CountWalls(MazeGrid* grid, int x, int y) {
    int count = 0;
    
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        MazeCell* neighbor = grid->GetNeighbor(x, y, dir);
        
        if (!neighbor || neighbor->GetType() == MazeCellType::Wall) {
            count++;
        }
    }
    
    return count;
}

std::vector<Direction> MazeGenerator::GetShuffledDirections() {
    std::vector<Direction> directions = {
        Direction::North,
        Direction::East,
        Direction::South,
        Direction::West
    };
    
    std::shuffle(directions.begin(), directions.end(), rng_);
    return directions;
}