/* Add in this module code that is specific to this core */

int matmul_unit(void);

int main(void)
{
	int status;

	status = 0;

	/* jump to multicore common code */
	matmul_unit();

	return status;
}
