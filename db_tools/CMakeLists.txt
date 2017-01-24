add_executable(db_gen db_gen.cpp)
function(db_gen_file name dir)
add_custom_command(OUTPUT	${dir}/${name}.cpp
				   COMMAND	rm -f ${dir}/${name}.h
				   COMMAND	rm -f ${dir}/${name}.cpp
				   COMMAND	db_gen -h ${dir}/${name}.json>>${dir}/${name}.h
				   COMMAND	db_gen -cpp ${dir}/${name}.json ${name}.h>>${dir}/${name}.cpp
				   DEPENDS	${dir}/${name}.json db_gen)
add_library(${name} ${dir}/${name}.cpp)
endfunction(db_gen_file)