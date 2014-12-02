#include <d7aoss.h>


static uint8_t buffer[128];

int main(void) {
	uint8_t ret;

	system_init(buffer, 128, buffer, 128);
	
	file_handler fh;

	fs_open(&fh, 0, file_system_user_user, file_system_access_type_read);

	if (fh.file != NULL)
		ret = fs_read_data(&fh, buffer, 0, 8);


	return ret;
}
