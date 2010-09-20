add_executable(test_H5xx
  H5xx.cpp
)
target_link_libraries(test_H5xx
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${HDF5_CPP_LIBRARY}
  ${HDF5_LIBRARY}
)
add_test(H5xx_attribute
  test_H5xx --run_test=H5xx_attribute
)
add_test(H5xx_dataset
  test_H5xx --run_test=H5xx_dataset
)
add_test(H5xx_group
  test_H5xx --run_test=H5xx_group
)
