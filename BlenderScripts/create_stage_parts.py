import bpy
import os

# パーツサイズの定義
GRID_SIZE = 4.0  # 4x4メートル
WALL_HEIGHT = 3.0  # 壁の高さ

def clear_scene():
    """シーンをクリア"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()

def create_floor_basic():
    """基本的な床タイルを作成"""
    clear_scene()
    
    # 床を作成
    bpy.ops.mesh.primitive_plane_add(size=GRID_SIZE, location=(0, 0, 0))
    floor = bpy.context.active_object
    floor.name = "floor_basic"
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Floor_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.3, 0.3, 0.3, 1.0)  # 暗めの灰色
    floor.data.materials.append(mat)
    
    return floor

def create_wall_straight():
    """直線壁を作成"""
    clear_scene()
    
    # 壁を作成（北側に配置）
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, GRID_SIZE/2 - 0.1, WALL_HEIGHT/2))
    wall = bpy.context.active_object
    wall.name = "wall_straight"
    wall.scale = (GRID_SIZE, 0.2, WALL_HEIGHT)
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Wall_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.5, 0.4, 0.3, 1.0)  # 茶色っぽい
    wall.data.materials.append(mat)
    
    return wall

def create_wall_corner():
    """コーナー壁を作成"""
    clear_scene()
    
    # L字型の壁を作成
    # 北側の壁
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, GRID_SIZE/2 - 0.1, WALL_HEIGHT/2))
    wall1 = bpy.context.active_object
    wall1.name = "wall_corner_north"
    wall1.scale = (GRID_SIZE, 0.2, WALL_HEIGHT)
    
    # 東側の壁
    bpy.ops.mesh.primitive_cube_add(size=1, location=(GRID_SIZE/2 - 0.1, 0, WALL_HEIGHT/2))
    wall2 = bpy.context.active_object
    wall2.name = "wall_corner_east"
    wall2.scale = (0.2, GRID_SIZE, WALL_HEIGHT)
    
    # 壁を結合
    wall1.select_set(True)
    wall2.select_set(True)
    bpy.context.view_layer.objects.active = wall1
    bpy.ops.object.join()
    
    wall1.name = "wall_corner"
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Wall_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.5, 0.4, 0.3, 1.0)
    wall1.data.materials.append(mat)
    
    return wall1

def create_wall_t_junction():
    """T字路の壁を作成"""
    clear_scene()
    
    # T字型の壁を作成
    # 北側の壁
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, GRID_SIZE/2 - 0.1, WALL_HEIGHT/2))
    wall1 = bpy.context.active_object
    wall1.scale = (GRID_SIZE, 0.2, WALL_HEIGHT)
    
    # 東側の壁（半分）
    bpy.ops.mesh.primitive_cube_add(size=1, location=(GRID_SIZE/2 - 0.1, -GRID_SIZE/4, WALL_HEIGHT/2))
    wall2 = bpy.context.active_object
    wall2.scale = (0.2, GRID_SIZE/2, WALL_HEIGHT)
    
    # 西側の壁（半分）
    bpy.ops.mesh.primitive_cube_add(size=1, location=(-GRID_SIZE/2 + 0.1, -GRID_SIZE/4, WALL_HEIGHT/2))
    wall3 = bpy.context.active_object
    wall3.scale = (0.2, GRID_SIZE/2, WALL_HEIGHT)
    
    # 壁を結合
    wall1.select_set(True)
    wall2.select_set(True)
    wall3.select_set(True)
    bpy.context.view_layer.objects.active = wall1
    bpy.ops.object.join()
    
    wall1.name = "wall_t_junction"
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Wall_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.5, 0.4, 0.3, 1.0)
    wall1.data.materials.append(mat)
    
    return wall1

def create_wall_door():
    """ドア付き壁を作成"""
    clear_scene()
    
    door_width = 1.5
    door_height = 2.2
    
    # 左側の壁
    bpy.ops.mesh.primitive_cube_add(size=1, location=(-(GRID_SIZE/4 + door_width/4), GRID_SIZE/2 - 0.1, WALL_HEIGHT/2))
    wall1 = bpy.context.active_object
    wall1.scale = ((GRID_SIZE - door_width)/2, 0.2, WALL_HEIGHT)
    
    # 右側の壁
    bpy.ops.mesh.primitive_cube_add(size=1, location=((GRID_SIZE/4 + door_width/4), GRID_SIZE/2 - 0.1, WALL_HEIGHT/2))
    wall2 = bpy.context.active_object
    wall2.scale = ((GRID_SIZE - door_width)/2, 0.2, WALL_HEIGHT)
    
    # 上部の壁（ドアの上）
    bpy.ops.mesh.primitive_cube_add(size=1, location=(0, GRID_SIZE/2 - 0.1, door_height + (WALL_HEIGHT - door_height)/2))
    wall3 = bpy.context.active_object
    wall3.scale = (door_width, 0.2, WALL_HEIGHT - door_height)
    
    # 壁を結合
    wall1.select_set(True)
    wall2.select_set(True)
    wall3.select_set(True)
    bpy.context.view_layer.objects.active = wall1
    bpy.ops.object.join()
    
    wall1.name = "wall_door"
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Wall_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.5, 0.4, 0.3, 1.0)
    wall1.data.materials.append(mat)
    
    return wall1

def create_ceiling_basic():
    """基本的な天井を作成"""
    clear_scene()
    
    # 天井を作成
    bpy.ops.mesh.primitive_plane_add(size=GRID_SIZE, location=(0, 0, WALL_HEIGHT))
    ceiling = bpy.context.active_object
    ceiling.name = "ceiling_basic"
    
    # 法線を下向きに
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.flip_normals()
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # マテリアルを追加
    mat = bpy.data.materials.new(name="Ceiling_Material")
    mat.use_nodes = True
    mat.node_tree.nodes["Principled BSDF"].inputs[0].default_value = (0.2, 0.2, 0.2, 1.0)  # 暗い灰色
    ceiling.data.materials.append(mat)
    
    return ceiling

def export_obj(obj, filename):
    """オブジェクトをOBJファイルとしてエクスポート"""
    # オブジェクトを選択
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    
    # エクスポート
    export_path = os.path.join("C:/Users/ryuto/OneDrive/ドキュメント/GitHub/MyEngineGame/Resources/Models/Stage", filename)
    bpy.ops.export_scene.obj(filepath=export_path, use_selection=True, use_materials=True)
    print(f"Exported: {export_path}")

# メイン処理
if __name__ == "__main__":
    # 各パーツを作成してエクスポート
    
    # 床
    floor = create_floor_basic()
    export_obj(floor, "Floor/floor_basic.obj")
    
    # 壁
    wall = create_wall_straight()
    export_obj(wall, "Wall/wall_straight.obj")
    
    corner = create_wall_corner()
    export_obj(corner, "Wall/wall_corner.obj")
    
    t_junction = create_wall_t_junction()
    export_obj(t_junction, "Wall/wall_t_junction.obj")
    
    door = create_wall_door()
    export_obj(door, "Wall/wall_door.obj")
    
    # 天井
    ceiling = create_ceiling_basic()
    export_obj(ceiling, "Ceiling/ceiling_basic.obj")
    
    print("All stage parts have been created and exported!")