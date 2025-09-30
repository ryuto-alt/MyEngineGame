"""
Blender Script for Dark Deception Style Maze Pieces Export
使用方法:
1. Blenderでこのスクリプトを実行
2. 迷路パーツをモデリング
3. 自動的にglTF形式でエクスポート
"""

import bpy
import os
from mathutils import Vector
import math

# エクスポート先のベースパス
EXPORT_BASE_PATH = "../../Resources/Models/maze/"

# グリッドサイズ定数
GRID_SIZE = 2.0
WALL_HEIGHT = 3.0
WALL_THICKNESS = 0.2

class MazePieceCreator:
    """迷路パーツ作成用クラス"""
    
    @staticmethod
    def create_floor_basic():
        """基本的な床タイルを作成"""
        bpy.ops.mesh.primitive_plane_add(size=GRID_SIZE, location=(0, 0, 0))
        floor = bpy.context.active_object
        floor.name = "floor_basic"
        
        # マテリアル設定
        mat = MazePieceCreator.create_dark_material("floor_mat", (0.2, 0.2, 0.2))
        floor.data.materials.append(mat)
        
        return floor
    
    @staticmethod
    def create_wall_straight():
        """直線壁を作成"""
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, 0, WALL_HEIGHT/2)
        )
        wall = bpy.context.active_object
        wall.name = "wall_straight"
        
        # スケール調整
        wall.scale = (GRID_SIZE, WALL_THICKNESS, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        
        # マテリアル設定
        mat = MazePieceCreator.create_dark_material("wall_mat", (0.15, 0.15, 0.15))
        wall.data.materials.append(mat)
        
        return wall
    
    @staticmethod
    def create_wall_corner():
        """L字コーナー壁を作成"""
        # 2つの壁を組み合わせてL字を作成
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(GRID_SIZE/2 - WALL_THICKNESS/2, 0, WALL_HEIGHT/2)
        )
        wall1 = bpy.context.active_object
        wall1.scale = (WALL_THICKNESS, GRID_SIZE, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, GRID_SIZE/2 - WALL_THICKNESS/2, WALL_HEIGHT/2)
        )
        wall2 = bpy.context.active_object
        wall2.scale = (GRID_SIZE, WALL_THICKNESS, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        
        # 結合
        wall1.select_set(True)
        wall2.select_set(True)
        bpy.context.view_layer.objects.active = wall1
        bpy.ops.object.join()
        
        wall1.name = "wall_corner_inner"
        
        # マテリアル設定
        mat = MazePieceCreator.create_dark_material("wall_corner_mat", (0.15, 0.15, 0.15))
        wall1.data.materials.append(mat)
        
        return wall1
    
    @staticmethod
    def create_wall_t_junction():
        """T字路壁を作成"""
        # 3つの壁を組み合わせてT字を作成
        walls = []
        
        # 縦壁
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(0, 0, WALL_HEIGHT/2)
        )
        wall = bpy.context.active_object
        wall.scale = (WALL_THICKNESS, GRID_SIZE, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        walls.append(wall)
        
        # 横壁（左）
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(-GRID_SIZE/2 + WALL_THICKNESS/2, GRID_SIZE/2 - WALL_THICKNESS/2, WALL_HEIGHT/2)
        )
        wall = bpy.context.active_object
        wall.scale = (GRID_SIZE/2, WALL_THICKNESS, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        walls.append(wall)
        
        # 横壁（右）
        bpy.ops.mesh.primitive_cube_add(
            size=1,
            location=(GRID_SIZE/2 - WALL_THICKNESS/2, GRID_SIZE/2 - WALL_THICKNESS/2, WALL_HEIGHT/2)
        )
        wall = bpy.context.active_object
        wall.scale = (GRID_SIZE/2, WALL_THICKNESS, WALL_HEIGHT)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        walls.append(wall)
        
        # 結合
        for w in walls:
            w.select_set(True)
        bpy.context.view_layer.objects.active = walls[0]
        bpy.ops.object.join()
        
        walls[0].name = "wall_t_junction"
        
        # マテリアル設定
        mat = MazePieceCreator.create_dark_material("wall_t_mat", (0.15, 0.15, 0.15))
        walls[0].data.materials.append(mat)
        
        return walls[0]
    
    @staticmethod
    def create_wall_cross():
        """十字路壁を作成"""
        walls = []
        
        # 4つのコーナー壁を作成
        positions = [
            (GRID_SIZE/2 - WALL_THICKNESS/2, GRID_SIZE/2 - WALL_THICKNESS/2),
            (-GRID_SIZE/2 + WALL_THICKNESS/2, GRID_SIZE/2 - WALL_THICKNESS/2),
            (GRID_SIZE/2 - WALL_THICKNESS/2, -GRID_SIZE/2 + WALL_THICKNESS/2),
            (-GRID_SIZE/2 + WALL_THICKNESS/2, -GRID_SIZE/2 + WALL_THICKNESS/2)
        ]
        
        for x, y in positions:
            bpy.ops.mesh.primitive_cube_add(
                size=1,
                location=(x, y, WALL_HEIGHT/2)
            )
            wall = bpy.context.active_object
            wall.scale = (WALL_THICKNESS, WALL_THICKNESS, WALL_HEIGHT)
            bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
            walls.append(wall)
        
        # 結合
        for w in walls:
            w.select_set(True)
        bpy.context.view_layer.objects.active = walls[0]
        bpy.ops.object.join()
        
        walls[0].name = "wall_cross"
        
        # マテリアル設定
        mat = MazePieceCreator.create_dark_material("wall_cross_mat", (0.15, 0.15, 0.15))
        walls[0].data.materials.append(mat)
        
        return walls[0]
    
    @staticmethod
    def create_flickering_light():
        """ちらつくライトを作成（エミッシブマテリアル付き）"""
        bpy.ops.mesh.primitive_cylinder_add(
            radius=0.05,
            depth=0.3,
            location=(0, 0, WALL_HEIGHT - 0.2)
        )
        light = bpy.context.active_object
        light.name = "light_flickering"
        
        # エミッシブマテリアル設定
        mat = bpy.data.materials.new(name="light_emissive_mat")
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        nodes.clear()
        
        # エミッションノード追加
        emission = nodes.new(type='ShaderNodeEmission')
        emission.inputs[0].default_value = (1.0, 0.9, 0.7, 1.0)  # 暖色系
        emission.inputs[1].default_value = 5.0  # 強度
        
        output = nodes.new(type='ShaderNodeOutputMaterial')
        mat.node_tree.links.new(emission.outputs[0], output.inputs[0])
        
        light.data.materials.append(mat)
        
        return light
    
    @staticmethod
    def create_dark_material(name, base_color):
        """Dark Deception風の暗いマテリアルを作成"""
        mat = bpy.data.materials.new(name=name)
        mat.use_nodes = True
        
        nodes = mat.node_tree.nodes
        bsdf = nodes.get("Principled BSDF")
        
        if bsdf:
            # Base Color
            bsdf.inputs[0].default_value = (*base_color, 1.0)
            # Metallic
            bsdf.inputs[6].default_value = 0.1
            # Roughness
            bsdf.inputs[9].default_value = 0.8
        
        return mat

class MazePieceExporter:
    """迷路パーツエクスポート用クラス"""
    
    @staticmethod
    def export_piece(obj, export_path):
        """単一のパーツをglTF形式でエクスポート"""
        # オブジェクトを選択
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        
        # エクスポートパスの作成
        full_path = os.path.join(export_path, f"{obj.name}.glb")
        os.makedirs(os.path.dirname(full_path), exist_ok=True)
        
        # glTFエクスポート設定
        bpy.ops.export_scene.gltf(
            filepath=full_path,
            export_format='GLB',
            use_selection=True,
            export_apply=True,
            export_texcoords=True,
            export_normals=True,
            export_materials='EXPORT',
            export_colors=True
        )
        
        print(f"Exported: {obj.name} to {full_path}")
    
    @staticmethod
    def export_all_pieces():
        """全ての迷路パーツをエクスポート"""
        export_path = os.path.abspath(EXPORT_BASE_PATH)
        
        # エクスポート対象のオブジェクトを収集
        pieces = [obj for obj in bpy.data.objects if obj.type == 'MESH' and 
                 (obj.name.startswith('floor_') or 
                  obj.name.startswith('wall_') or 
                  obj.name.startswith('light_'))]
        
        for piece in pieces:
            MazePieceExporter.export_piece(piece, export_path)
        
        print(f"Exported {len(pieces)} pieces to {export_path}")

def create_all_pieces():
    """全ての基本パーツを作成"""
    # 既存のオブジェクトをクリア
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()
    
    # パーツ作成
    pieces = []
    pieces.append(MazePieceCreator.create_floor_basic())
    pieces.append(MazePieceCreator.create_wall_straight())
    pieces.append(MazePieceCreator.create_wall_corner())
    pieces.append(MazePieceCreator.create_wall_t_junction())
    pieces.append(MazePieceCreator.create_wall_cross())
    pieces.append(MazePieceCreator.create_flickering_light())
    
    # 配置調整（見やすくするため）
    for i, piece in enumerate(pieces):
        piece.location.x = i * (GRID_SIZE + 0.5)
    
    return pieces

def setup_scene():
    """シーンの初期設定"""
    # 単位をメートルに設定
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1.0
    
    # グリッド設定
    for area in bpy.context.screen.areas:
        if area.type == 'VIEW_3D':
            for space in area.spaces:
                if space.type == 'VIEW_3D':
                    space.overlay.grid_scale = GRID_SIZE
                    space.overlay.grid_subdivisions = 10

# メイン実行部分
if __name__ == "__main__":
    setup_scene()
    pieces = create_all_pieces()
    
    # エクスポート実行（コメントアウトを解除して実行）
    # MazePieceExporter.export_all_pieces()
    
    print("Maze pieces created successfully!")
    print("To export, uncomment the export line in the script.")