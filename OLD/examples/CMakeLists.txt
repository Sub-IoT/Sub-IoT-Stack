ADD_SUBDIRECTORY(star_push_endpoint_example)
ADD_SUBDIRECTORY(star_push_gateway_example)

IF(D7AOSS_HAL_DRIVER STREQUAL "cc430")
	# TODO CC430 specific for now
	ADD_SUBDIRECTORY(request_response_example_requester)
	ADD_SUBDIRECTORY(request_response_example_resp)
ADD_SUBDIRECTORY(file_notification)
ADD_SUBDIRECTORY(phy_logger)
ENDIF()
