# Export data from blender

import bpy
import json

def item(collection:str):
    print(json.dumps({
        collection.lower(): [{
            'label': o.name,
            'pos': [round(o.location[0], 3), round(o.location[2], 3), -round(o.location[1], 3)]
        } for o in bpy.data.collections[collection].objects]
    }, ensure_ascii = False))

item('Accidents')
item('Cameras')
item('Parking')
item('Persons')
item('Shops')