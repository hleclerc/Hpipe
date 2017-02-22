cmake_minimum_required(VERSION 3.0)
project( hpipe )

include_directories( src ext/PrepArg/src )

add_executable( hpipe
    src/Hpipe/main.cpp
    src/Hpipe/Assert.cpp
    src/Hpipe/BranchSet.cpp
    src/Hpipe/CbQueue.cpp
    src/Hpipe/CbString.cpp
    src/Hpipe/CbStringPtr.cpp
    src/Hpipe/CharEdge.cpp
    src/Hpipe/CharGraph.cpp
    src/Hpipe/CharItem.cpp
    src/Hpipe/CmString.cpp
    src/Hpipe/Cond.cpp
    src/Hpipe/Context.cpp
    src/Hpipe/CppEmitter.cpp
    src/Hpipe/DaSi.cpp
    src/Hpipe/DotOut.cpp
    src/Hpipe/ErrorList.cpp
    src/Hpipe/FindVarInCode.cpp
    src/Hpipe/Instruction.cpp
    src/Hpipe/InstructionAddStr.cpp
    src/Hpipe/InstructionClrStr.cpp
    src/Hpipe/InstructionCode.cpp
    src/Hpipe/InstructionCond.cpp
    src/Hpipe/InstructionEof.cpp
    src/Hpipe/InstructionGraph.cpp
    src/Hpipe/InstructionKO.cpp
    src/Hpipe/InstructionMark.cpp
    src/Hpipe/InstructionMultiCond.cpp
    src/Hpipe/InstructionNextChar.cpp
    src/Hpipe/InstructionNone.cpp
    src/Hpipe/InstructionOK.cpp
    src/Hpipe/InstructionPrio.cpp
    src/Hpipe/InstructionRewind.cpp
    src/Hpipe/InstructionSave.cpp
    src/Hpipe/InstructionSkip.cpp
    src/Hpipe/InstructionIf.cpp
    src/Hpipe/InstructionTestContiguous.cpp
    src/Hpipe/InstructionWithCode.cpp
    src/Hpipe/Lexem.cpp
    src/Hpipe/Lexer.cpp
    src/Hpipe/Source.cpp
    src/Hpipe/Transition.cpp
    src/Hpipe/predef.h
)

set_property( TARGET hpipe PROPERTY CXX_STANDARD 14 )

# INSTALL
install(
  TARGETS
    hpipe
  DESTINATION
    bin
)

install (
  FILES
    src/Hpipe/Assert.h
    src/Hpipe/AutoPtr.h
    src/Hpipe/Buffer.h
    src/Hpipe/CbQueue.h
    src/Hpipe/CbString.h
    src/Hpipe/CbStringPtr.h
    src/Hpipe/CfQueue.h
    src/Hpipe/Check.h
    src/Hpipe/CmQueue.h
    src/Hpipe/CmString.h
    src/Hpipe/DaSi.h
    src/Hpipe/EnableIf.h
    src/Hpipe/inc_and_dec_ref.h
    src/Hpipe/Pool.h
    src/Hpipe/Print.h
    src/Hpipe/PtrPool.h
    src/Hpipe/Stream.h
    src/Hpipe/StreamSep.h
    src/Hpipe/TypeConfig.h
    src/Hpipe/Vec.h
    src/Hpipe/WriteAs.h
  DESTINATION
    include/Hpipe
)
         
# TST
include( CTest )

macro( test_hpipe_file filename )
  add_test( test_hpipe_file_${filename} ${CMAKE_BINARY_DIR}/hpipe -t ${CMAKE_SOURCE_DIR}/${filename} )
  set_tests_properties( test_hpipe_file_${filename} PROPERTIES FAIL_REGULAR_EXPRESSION "(BAD)" PASS_REGULAR_EXPRESSION "(OK)" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )
endmacro( test_hpipe_file )
 
test_hpipe_file( tests/hpipe/simple_loop.hpipe      )
test_hpipe_file( tests/hpipe/multi_loop.hpipe       )
test_hpipe_file( tests/hpipe/machine_argument.hpipe )
test_hpipe_file( tests/hpipe/eof.hpipe              )
test_hpipe_file( tests/hpipe/eof_2.hpipe            )
test_hpipe_file( tests/hpipe/goto.hpipe             )
test_hpipe_file( tests/hpipe/cpp_preproc.hpipe      )
test_hpipe_file( tests/hpipe/mark_1_simple.hpipe    )
test_hpipe_file( tests/hpipe/mark_2_loop.hpipe      )
test_hpipe_file( tests/hpipe/mark_3_rec.hpipe       )
test_hpipe_file( tests/hpipe/mark_4_bis.hpipe       )
test_hpipe_file( tests/hpipe/mark_4_unused.hpipe    )
test_hpipe_file( tests/hpipe/mark_5_rec.hpipe       )
test_hpipe_file( tests/hpipe/number.hpipe           )
test_hpipe_file( tests/hpipe/opt_beg_end.hpipe      )
test_hpipe_file( tests/hpipe/ast.hpipe              )
test_hpipe_file( tests/hpipe/cpp_char.hpipe         )
test_hpipe_file( tests/hpipe/c_comment.hpipe        )

# benchmarks
# test_hpipe_file( tests/hpipe/boyer_moore.hpipe      )
# test_hpipe_file( tests/hpipe/cpp_bench.hpipe        )
# test_hpipe_file( tests/hpipe/training.hpipe         )

add_custom_target( bench_boyer_moore_off COMMAND hpipe -b --no-boyer-moore ${CMAKE_SOURCE_DIR}/tests/hpipe/boyer_moore.hpipe )
add_custom_target( bench_boyer_moore_on  COMMAND hpipe -b                  ${CMAKE_SOURCE_DIR}/tests/hpipe/boyer_moore.hpipe )
add_custom_target( bench_training        COMMAND hpipe -b                  ${CMAKE_SOURCE_DIR}/tests/hpipe/training.hpipe    )

# generated file
add_executable(
  txt_to_cpp
  src/txt_to_cpp.cpp
)
set_property( TARGET txt_to_cpp PROPERTY CXX_STANDARD 14 )

add_custom_command( OUTPUT  ${CMAKE_SOURCE_DIR}/src/Hpipe/predef.h
                    COMMAND txt_to_cpp ${CMAKE_SOURCE_DIR}/src/Hpipe/predef.h ${CMAKE_SOURCE_DIR}/src/Hpipe/predef.hpipe predef
                    DEPENDS src/Hpipe/predef.hpipe txt_to_cpp )

