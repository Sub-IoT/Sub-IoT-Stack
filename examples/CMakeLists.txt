ADD_SUBDIRECTORY(star_push_endpoint_example)
ADD_SUBDIRECTORY(star_push_gateway_example)

# TODO CC430 specific for now
IF(D7AOSS_HAL_DRIVER STREQUAL "cc430")
	ADD_SUBDIRECTORY(request_response_example_requester)
	ADD_SUBDIRECTORY(request_response_example_resp)
ENDIF()