add_executable(test_h5xx
  h5xx.cpp
)
target_link_libraries(test_h5xx
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${HDF5_CPP_LIBRARY}
  ${HDF5_LIBRARY}
)
add_test(h5xx_attribute
  test_h5xx --run_test=h5xx_attribute --log_level=test_suite
)
add_test(h5xx_dataset
  test_h5xx --run_test=h5xx_dataset --log_level=test_suite
)
add_test(h5xx_unique_dataset
  test_h5xx --run_test=h5xx_unique_dataset --log_level=test_suite
)
add_test(h5xx_group
  test_h5xx --run_test=h5xx_group --log_level=test_suite
)
