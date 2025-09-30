#!/usr/bin/env python3
"""
Dark Deception Style Maze Editor
迷路を生成・編集・保存するためのツール
"""

import json
import random
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from enum import Enum
import os
from typing import List, Tuple, Optional

class CellType(Enum):
    EMPTY = 0
    WALL = 1
    FLOOR = 2
    START = 3
    GOAL = 4
    ITEM = 5
    ENEMY = 6
    PORTAL = 7
    DOOR = 8
    KEY = 9

class Direction(Enum):
    NORTH = 0
    EAST = 1
    SOUTH = 2
    WEST = 3

class MazeCell:
    def __init__(self):
        self.type = CellType.EMPTY
        self.walls = [True, True, True, True]  # N, E, S, W
        self.visited = False
        self.darkness = 0.0
        self.fog_density = 0.0

class MazeGrid:
    def __init__(self, width: int, height: int, cell_size: float = 2.0):
        self.width = width
        self.height = height
        self.cell_size = cell_size
        self.cells = [[MazeCell() for _ in range(width)] for _ in range(height)]
        self.start_pos = (0, 0)
        self.goal_pos = (width - 1, height - 1)

    def get_cell(self, x: int, y: int) -> Optional[MazeCell]:
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.cells[y][x]
        return None

    def get_neighbors(self, x: int, y: int) -> List[Tuple[int, int, Direction]]:
        neighbors = []
        directions = [
            (0, -1, Direction.NORTH),
            (1, 0, Direction.EAST),
            (0, 1, Direction.SOUTH),
            (-1, 0, Direction.WEST)
        ]
        for dx, dy, direction in directions:
            nx, ny = x + dx, y + dy
            if 0 <= nx < self.width and 0 <= ny < self.height:
                neighbors.append((nx, ny, direction))
        return neighbors

    def export_to_json(self) -> str:
        data = {
            "width": self.width,
            "height": self.height,
            "cellSize": self.cell_size,
            "startPos": list(self.start_pos),
            "goalPos": list(self.goal_pos),
            "cells": []
        }
        
        for row in self.cells:
            cell_row = []
            for cell in row:
                cell_data = {
                    "type": cell.type.value,
                    "walls": cell.walls,
                    "darkness": cell.darkness,
                    "fogDensity": cell.fog_density
                }
                cell_row.append(cell_data)
            data["cells"].append(cell_row)
        
        return json.dumps(data, indent=2)

    def import_from_json(self, json_str: str):
        data = json.loads(json_str)
        self.width = data["width"]
        self.height = data["height"]
        self.cell_size = data["cellSize"]
        self.start_pos = tuple(data["startPos"])
        self.goal_pos = tuple(data["goalPos"])
        
        self.cells = [[MazeCell() for _ in range(self.width)] for _ in range(self.height)]
        
        for y, row_data in enumerate(data["cells"]):
            for x, cell_data in enumerate(row_data):
                cell = self.cells[y][x]
                cell.type = CellType(cell_data["type"])
                cell.walls = cell_data.get("walls", [True, True, True, True])
                cell.darkness = cell_data.get("darkness", 0.0)
                cell.fog_density = cell_data.get("fogDensity", 0.0)

class MazeGenerator:
    @staticmethod
    def generate_recursive_backtracking(grid: MazeGrid):
        """再帰的バックトラッキングアルゴリズムで迷路を生成"""
        # 全セルを壁で初期化
        for y in range(grid.height):
            for x in range(grid.width):
                grid.cells[y][x].type = CellType.WALL
                grid.cells[y][x].visited = False
        
        stack = []
        current = (0, 0)
        grid.cells[0][0].type = CellType.FLOOR
        grid.cells[0][0].visited = True
        stack.append(current)
        
        while stack:
            x, y = current
            # 未訪問の隣接セルを探す
            unvisited = []
            for nx, ny, direction in grid.get_neighbors(x, y):
                # 1マス飛ばしで確認（壁を挟むため）
                if nx % 2 == 0 and ny % 2 == 0:
                    if not grid.cells[ny][nx].visited:
                        unvisited.append((nx, ny, direction))
            
            if unvisited:
                # ランダムに次のセルを選択
                nx, ny, direction = random.choice(unvisited)
                
                # 現在のセルと次のセルの間の壁を削除
                wall_x = x + (nx - x) // 2
                wall_y = y + (ny - y) // 2
                grid.cells[wall_y][wall_x].type = CellType.FLOOR
                
                # 次のセルを通路にする
                grid.cells[ny][nx].type = CellType.FLOOR
                grid.cells[ny][nx].visited = True
                
                stack.append((nx, ny))
                current = (nx, ny)
            else:
                if stack:
                    current = stack.pop()
        
        # スタートとゴールを設定
        grid.cells[grid.start_pos[1]][grid.start_pos[0]].type = CellType.START
        grid.cells[grid.goal_pos[1]][grid.goal_pos[0]].type = CellType.GOAL

    @staticmethod
    def generate_dark_deception_style(grid: MazeGrid):
        """Dark Deception風の迷路を生成（長い通路、少ない分岐）"""
        # 全セルを壁で初期化
        for y in range(grid.height):
            for x in range(grid.width):
                grid.cells[y][x].type = CellType.WALL
        
        # メインパスを作成（スネーク状）
        x, y = 0, 0
        grid.cells[y][x].type = CellType.FLOOR
        
        while x < grid.width - 1 or y < grid.height - 1:
            if x >= grid.width - 1:
                if y < grid.height - 1:
                    y += 1
            elif y >= grid.height - 1:
                if x < grid.width - 1:
                    x += 1
            else:
                # 70%の確率で右、30%の確率で下
                if random.random() < 0.7 and x < grid.width - 1:
                    x += 1
                elif y < grid.height - 1:
                    y += 1
                else:
                    x += 1
            
            if x < grid.width and y < grid.height:
                grid.cells[y][x].type = CellType.FLOOR
        
        # 分岐を追加
        for _ in range(grid.width * grid.height // 20):  # 少なめの分岐
            x = random.randint(1, grid.width - 2)
            y = random.randint(1, grid.height - 2)
            
            if grid.cells[y][x].type == CellType.FLOOR:
                # ランダムな方向に通路を延ばす
                direction = random.choice([(0, 1), (1, 0), (0, -1), (-1, 0)])
                length = random.randint(3, 8)
                
                for i in range(length):
                    nx = x + direction[0] * (i + 1)
                    ny = y + direction[1] * (i + 1)
                    
                    if 0 < nx < grid.width - 1 and 0 < ny < grid.height - 1:
                        grid.cells[ny][nx].type = CellType.FLOOR
        
        # 部屋を追加
        for _ in range(3):  # 3つの部屋
            room_width = random.randint(3, 5)
            room_height = random.randint(3, 5)
            room_x = random.randint(1, grid.width - room_width - 1)
            room_y = random.randint(1, grid.height - room_height - 1)
            
            for ry in range(room_height):
                for rx in range(room_width):
                    if room_y + ry < grid.height and room_x + rx < grid.width:
                        grid.cells[room_y + ry][room_x + rx].type = CellType.FLOOR
        
        # ダークゾーンを追加
        for _ in range(5):
            zone_width = random.randint(3, 6)
            zone_height = random.randint(3, 6)
            zone_x = random.randint(0, max(0, grid.width - zone_width))
            zone_y = random.randint(0, max(0, grid.height - zone_height))
            darkness = random.uniform(0.3, 0.8)
            
            for zy in range(zone_height):
                for zx in range(zone_width):
                    if zone_y + zy < grid.height and zone_x + zx < grid.width:
                        grid.cells[zone_y + zy][zone_x + zx].darkness = darkness
        
        # スタートとゴールを設定
        grid.cells[grid.start_pos[1]][grid.start_pos[0]].type = CellType.START
        grid.cells[grid.goal_pos[1]][grid.goal_pos[0]].type = CellType.GOAL

class MazeEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("Dark Deception Maze Editor")
        self.root.geometry("1200x800")
        
        self.grid_width = 20
        self.grid_height = 20
        self.cell_size = 2.0
        self.grid = MazeGrid(self.grid_width, self.grid_height, self.cell_size)
        self.selected_tool = CellType.FLOOR
        self.canvas_cell_size = 20  # キャンバス上のセルサイズ
        
        self.setup_ui()
        self.redraw_canvas()

    def setup_ui(self):
        # メニューバー
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="New", command=self.new_maze)
        file_menu.add_command(label="Open...", command=self.load_maze)
        file_menu.add_command(label="Save...", command=self.save_maze)
        file_menu.add_separator()
        file_menu.add_command(label="Export to JSON...", command=self.export_json)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.root.quit)
        
        generate_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Generate", menu=generate_menu)
        generate_menu.add_command(label="Recursive Backtracking", 
                                 command=lambda: self.generate_maze("backtracking"))
        generate_menu.add_command(label="Dark Deception Style", 
                                 command=lambda: self.generate_maze("dark_deception"))
        generate_menu.add_command(label="Clear All", command=self.clear_maze)
        
        # ツールバー
        toolbar = ttk.Frame(self.root)
        toolbar.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)
        
        ttk.Label(toolbar, text="Tool:").pack(side=tk.LEFT, padx=5)
        
        self.tool_var = tk.StringVar(value="Floor")
        tool_combo = ttk.Combobox(toolbar, textvariable=self.tool_var, width=15)
        tool_combo['values'] = ["Empty", "Wall", "Floor", "Start", "Goal", "Item", "Enemy", "Portal", "Door", "Key"]
        tool_combo.pack(side=tk.LEFT, padx=5)
        tool_combo.bind('<<ComboboxSelected>>', self.on_tool_change)
        
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        ttk.Label(toolbar, text="Grid Size:").pack(side=tk.LEFT, padx=5)
        
        self.width_var = tk.IntVar(value=self.grid_width)
        width_spin = ttk.Spinbox(toolbar, from_=10, to=100, textvariable=self.width_var, width=10)
        width_spin.pack(side=tk.LEFT, padx=2)
        
        ttk.Label(toolbar, text="x").pack(side=tk.LEFT)
        
        self.height_var = tk.IntVar(value=self.grid_height)
        height_spin = ttk.Spinbox(toolbar, from_=10, to=100, textvariable=self.height_var, width=10)
        height_spin.pack(side=tk.LEFT, padx=2)
        
        ttk.Button(toolbar, text="Resize", command=self.resize_grid).pack(side=tk.LEFT, padx=5)
        
        ttk.Separator(toolbar, orient=tk.VERTICAL).pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        ttk.Label(toolbar, text="Cell Size:").pack(side=tk.LEFT, padx=5)
        self.cell_size_var = tk.DoubleVar(value=self.cell_size)
        cell_size_spin = ttk.Spinbox(toolbar, from_=1.0, to=10.0, increment=0.5, 
                                     textvariable=self.cell_size_var, width=10)
        cell_size_spin.pack(side=tk.LEFT, padx=2)
        
        # キャンバス
        canvas_frame = ttk.Frame(self.root)
        canvas_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        self.canvas = tk.Canvas(canvas_frame, bg="black", 
                               width=self.grid_width * self.canvas_cell_size,
                               height=self.grid_height * self.canvas_cell_size)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # スクロールバー
        v_scrollbar = ttk.Scrollbar(canvas_frame, orient=tk.VERTICAL, command=self.canvas.yview)
        v_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.canvas.config(yscrollcommand=v_scrollbar.set)
        
        h_scrollbar = ttk.Scrollbar(self.root, orient=tk.HORIZONTAL, command=self.canvas.xview)
        h_scrollbar.pack(side=tk.BOTTOM, fill=tk.X)
        self.canvas.config(xscrollcommand=h_scrollbar.set)
        
        # マウスイベント
        self.canvas.bind("<Button-1>", self.on_canvas_click)
        self.canvas.bind("<B1-Motion>", self.on_canvas_drag)
        self.canvas.bind("<Button-3>", self.on_canvas_right_click)
        
        # ステータスバー
        self.status_bar = ttk.Label(self.root, text="Ready", relief=tk.SUNKEN, anchor=tk.W)
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)

    def on_tool_change(self, event):
        tool_map = {
            "Empty": CellType.EMPTY,
            "Wall": CellType.WALL,
            "Floor": CellType.FLOOR,
            "Start": CellType.START,
            "Goal": CellType.GOAL,
            "Item": CellType.ITEM,
            "Enemy": CellType.ENEMY,
            "Portal": CellType.PORTAL,
            "Door": CellType.DOOR,
            "Key": CellType.KEY
        }
        self.selected_tool = tool_map[self.tool_var.get()]

    def on_canvas_click(self, event):
        self.paint_cell(event.x, event.y)

    def on_canvas_drag(self, event):
        self.paint_cell(event.x, event.y)

    def on_canvas_right_click(self, event):
        x = event.x // self.canvas_cell_size
        y = event.y // self.canvas_cell_size
        
        if 0 <= x < self.grid.width and 0 <= y < self.grid.height:
            cell = self.grid.cells[y][x]
            self.show_cell_properties(x, y, cell)

    def paint_cell(self, canvas_x, canvas_y):
        x = canvas_x // self.canvas_cell_size
        y = canvas_y // self.canvas_cell_size
        
        if 0 <= x < self.grid.width and 0 <= y < self.grid.height:
            # スタートとゴールは1つだけ
            if self.selected_tool == CellType.START:
                # 既存のスタートを削除
                for row in self.grid.cells:
                    for cell in row:
                        if cell.type == CellType.START:
                            cell.type = CellType.FLOOR
                self.grid.start_pos = (x, y)
            elif self.selected_tool == CellType.GOAL:
                # 既存のゴールを削除
                for row in self.grid.cells:
                    for cell in row:
                        if cell.type == CellType.GOAL:
                            cell.type = CellType.FLOOR
                self.grid.goal_pos = (x, y)
            
            self.grid.cells[y][x].type = self.selected_tool
            self.redraw_cell(x, y)
            self.update_status(f"Painted cell ({x}, {y}) with {self.selected_tool.name}")

    def show_cell_properties(self, x, y, cell):
        """セルのプロパティウィンドウを表示"""
        prop_window = tk.Toplevel(self.root)
        prop_window.title(f"Cell Properties ({x}, {y})")
        prop_window.geometry("300x200")
        
        ttk.Label(prop_window, text=f"Position: ({x}, {y})").pack(pady=5)
        ttk.Label(prop_window, text=f"Type: {cell.type.name}").pack(pady=5)
        
        ttk.Label(prop_window, text="Darkness:").pack(pady=5)
        darkness_var = tk.DoubleVar(value=cell.darkness)
        darkness_scale = ttk.Scale(prop_window, from_=0.0, to=1.0, variable=darkness_var, orient=tk.HORIZONTAL)
        darkness_scale.pack(fill=tk.X, padx=20)
        
        ttk.Label(prop_window, text="Fog Density:").pack(pady=5)
        fog_var = tk.DoubleVar(value=cell.fog_density)
        fog_scale = ttk.Scale(prop_window, from_=0.0, to=1.0, variable=fog_var, orient=tk.HORIZONTAL)
        fog_scale.pack(fill=tk.X, padx=20)
        
        def apply_properties():
            cell.darkness = darkness_var.get()
            cell.fog_density = fog_var.get()
            self.redraw_cell(x, y)
            prop_window.destroy()
        
        ttk.Button(prop_window, text="Apply", command=apply_properties).pack(pady=10)

    def redraw_canvas(self):
        self.canvas.delete("all")
        
        # キャンバスサイズを更新
        canvas_width = self.grid.width * self.canvas_cell_size
        canvas_height = self.grid.height * self.canvas_cell_size
        self.canvas.config(scrollregion=(0, 0, canvas_width, canvas_height))
        
        # グリッドを描画
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                self.redraw_cell(x, y)

    def redraw_cell(self, x, y):
        cell = self.grid.cells[y][x]
        
        # セルの色を決定
        color_map = {
            CellType.EMPTY: "#000000",
            CellType.WALL: "#404040",
            CellType.FLOOR: "#808080",
            CellType.START: "#00FF00",
            CellType.GOAL: "#FF0000",
            CellType.ITEM: "#FFFF00",
            CellType.ENEMY: "#FF00FF",
            CellType.PORTAL: "#00FFFF",
            CellType.DOOR: "#8B4513",
            CellType.KEY: "#FFD700"
        }
        
        color = color_map[cell.type]
        
        # 暗さを適用
        if cell.darkness > 0:
            # 暗さに応じて色を暗くする
            r = int(color[1:3], 16)
            g = int(color[3:5], 16)
            b = int(color[5:7], 16)
            factor = 1.0 - cell.darkness * 0.5
            r = int(r * factor)
            g = int(g * factor)
            b = int(b * factor)
            color = f"#{r:02x}{g:02x}{b:02x}"
        
        # セルを描画
        x1 = x * self.canvas_cell_size
        y1 = y * self.canvas_cell_size
        x2 = x1 + self.canvas_cell_size
        y2 = y1 + self.canvas_cell_size
        
        self.canvas.create_rectangle(x1, y1, x2, y2, fill=color, outline="gray", tags=f"cell_{x}_{y}")

    def new_maze(self):
        if messagebox.askyesno("New Maze", "Create a new maze? Current maze will be lost."):
            self.grid = MazeGrid(self.grid_width, self.grid_height, self.cell_size)
            self.redraw_canvas()
            self.update_status("New maze created")

    def clear_maze(self):
        for y in range(self.grid.height):
            for x in range(self.grid.width):
                self.grid.cells[y][x].type = CellType.EMPTY
                self.grid.cells[y][x].darkness = 0.0
                self.grid.cells[y][x].fog_density = 0.0
        self.redraw_canvas()
        self.update_status("Maze cleared")

    def generate_maze(self, algorithm):
        if algorithm == "backtracking":
            MazeGenerator.generate_recursive_backtracking(self.grid)
        elif algorithm == "dark_deception":
            MazeGenerator.generate_dark_deception_style(self.grid)
        
        self.redraw_canvas()
        self.update_status(f"Generated maze using {algorithm}")

    def resize_grid(self):
        new_width = self.width_var.get()
        new_height = self.height_var.get()
        new_cell_size = self.cell_size_var.get()
        
        if messagebox.askyesno("Resize Grid", 
                               f"Resize grid to {new_width}x{new_height}? Current maze will be lost."):
            self.grid_width = new_width
            self.grid_height = new_height
            self.cell_size = new_cell_size
            self.grid = MazeGrid(self.grid_width, self.grid_height, self.cell_size)
            self.redraw_canvas()
            self.update_status(f"Grid resized to {new_width}x{new_height}")

    def save_maze(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".maze",
            filetypes=[("Maze files", "*.maze"), ("All files", "*.*")]
        )
        
        if file_path:
            try:
                with open(file_path, 'w') as f:
                    f.write(self.grid.export_to_json())
                self.update_status(f"Saved maze to {os.path.basename(file_path)}")
            except Exception as e:
                messagebox.showerror("Save Error", f"Failed to save maze: {e}")

    def load_maze(self):
        file_path = filedialog.askopenfilename(
            filetypes=[("Maze files", "*.maze"), ("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if file_path:
            try:
                with open(file_path, 'r') as f:
                    json_str = f.read()
                    self.grid.import_from_json(json_str)
                    self.grid_width = self.grid.width
                    self.grid_height = self.grid.height
                    self.width_var.set(self.grid_width)
                    self.height_var.set(self.grid_height)
                    self.cell_size_var.set(self.grid.cell_size)
                    self.redraw_canvas()
                    self.update_status(f"Loaded maze from {os.path.basename(file_path)}")
            except Exception as e:
                messagebox.showerror("Load Error", f"Failed to load maze: {e}")

    def export_json(self):
        file_path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")]
        )
        
        if file_path:
            try:
                with open(file_path, 'w') as f:
                    f.write(self.grid.export_to_json())
                self.update_status(f"Exported maze to {os.path.basename(file_path)}")
            except Exception as e:
                messagebox.showerror("Export Error", f"Failed to export maze: {e}")

    def update_status(self, message):
        self.status_bar.config(text=message)

def main():
    root = tk.Tk()
    app = MazeEditor(root)
    root.mainloop()

if __name__ == "__main__":
    main()