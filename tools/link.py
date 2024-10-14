# Export connections from blender
# Usage:
# 1. Select multiple objects. This will be you roads markers
# 2. Make one object active. This will be a node that other markers connected with
# 3. Run the script and check console

import bpy

import json

name = lambda n: int(n.replace('Empty.', ''))

active = bpy.context.active_object
print(json.dumps({
    'node': name(active.name),
    'conn': [{
        'c': name(c.name),
        'w': round((active.location - c.location).length, 3)
    } for c in bpy.context.selected_objects if c is not active]
}))