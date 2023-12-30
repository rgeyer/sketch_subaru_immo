import os
import shutil

def renameFile(input_file,extension):
    try:
        os.rename("CAMOutputs/"+input_file,"CAMOutputs/"+extension)
        print("{} renamed to {}.".format(input_file,extension))
    except:
        print("{} is missing.".format(input_file))



try:
    shutil.rmtree("CAMOutputs/Assembly")
except:
    pass
try:
    for i in os.listdir("CAMOutputs/DrillFiles/"):
        shutil.move(os.path.join("CAMOutputs/DrillFiles/", i), "CAMOutputs")
    shutil.rmtree("CAMOutputs/DrillFiles/")
except:
    pass
try:
    for i in os.listdir("CAMOutputs/GerberFiles/"):
        shutil.move(os.path.join("CAMOutputs/GerberFiles/", i), "CAMOutputs")
    shutil.rmtree("CAMOutputs/GerberFiles")
except:
    pass

renameFile("copper_bottom.gbr","copper_bottom.gbl")
renameFile("silkscreen_bottom.gbr","silkscreen_bottom.gbo")
renameFile("soldermask_bottom.gbr","soldermask_bottom.gbs")
renameFile("copper_top.gbr","copper_top.gtl")
renameFile("silkscreen_top.gbr","silkscreen_top.gto")
renameFile("soldermask_top.gbr","soldermask_top.gts")
renameFile("solderpaste_top.gbr","solderpaste_top.gtp")
renameFile("profile.gbr","profile.gko")



