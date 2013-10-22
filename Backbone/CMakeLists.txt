set(backbone_source_files
	Algorithm.hpp
	Backbone.hpp
	BackStore.hpp
	BackStore.cpp
	IMGData.cpp
	IMGData.hpp
	)
set(dist_source_files
	Algs.hpp
	Maps.hpp
	Maps.cpp
	DistWorker.cpp
	)
set(push_source_files
	PushWorker.cpp
	)
set(server_source_files
	Server.cpp
	)

add_library(backbone ${backbone_source_files})
add_library(dist ${dist_source_files})
add_library(push ${push_source_files})
add_library(server ${server_source_files})

add_executable(zServer ${server_source_files})
add_executable(zPushWorker ${push_source_files})
add_executable(zDistWorker ${dist_source_files})

target_link_libraries(
	zDistWorker
	dist
	ogrsaliency
	recognition
	display
	guirec
	guisaliency
	imagepub
	imagepush
	publish
	verification
	)
target_link_libraries(
	zServer
	server
	push
	backbone
	)
