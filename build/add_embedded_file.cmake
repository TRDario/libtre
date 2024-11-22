find_program(XXD xxd)

function(add_embedded_file TARGET FILE ARRAYNAME)
	add_custom_command(
		OUTPUT ${FILE}.hpp
		COMMAND ${XXD} -i -n ${ARRAYNAME} ${FILE} ${FILE}.hpp
		DEPENDS ${FILE}
		COMMENT "Creating embeddable header ${FILE}.hpp"
		VERBATIM
	)
	target_sources(tre PRIVATE ${FILE}.hpp)
endfunction()