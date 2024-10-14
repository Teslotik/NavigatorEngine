import bpy
import json

print(json.dumps({
    'panoramas': [{
        'label': o.name,
        'pos': [round(o.location[0], 3), round(o.location[2], 3), -round(o.location[1], 3)],
        'rot': [round(o.rotation_euler[0], 3), round(o.rotation_euler[2], 3), round(o.rotation_euler[1], 3)]
    } for o in bpy.data.collections['Panoramas'].objects]
}, ensure_ascii = False))