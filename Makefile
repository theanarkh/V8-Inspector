build:
	g++ -g -Iyour_v8_header_dir_path src/*.cc src/inspector/*.cc -o No -lv8_monolith -ldl -Lyour_v8_lib_dir_path -pthread -std=c++14 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX