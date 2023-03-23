import os

proj_dir = os.getcwd()
main_dir = os.path.dirname(proj_dir)
dir_name = os.path.basename(proj_dir)
gcannon_path = main_dir + '\\gophercan-lib\\network_autogen'
car_path = gcannon_path + '\\configs\\dyno-can-config.yaml'
os.chdir(gcannon_path)
os.system('python ' + 'autogen.py' + ' ' + car_path)
