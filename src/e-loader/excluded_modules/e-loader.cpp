#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include <sys/mman.h>
#include <fcntl.h>
#include <err.h>

#include "e-loader.h"
#include "e-host.h"

#define diag(N)   if (sverbose >= N)

using namespace std;

vector<unsigned> fDiscoveredCoreIds;

static int sverbose;

int e_load(char *srecFile, bool reset_target, bool broadcast, bool run_target, int verbose)
{
	int status = EPI_OK;

	sverbose = verbose;

	Epiphany_t *pEpiphany;

	pEpiphany = new Epiphany_t;

	if (pEpiphany) {
		e_open(pEpiphany);

		if (reset_target) {
			diag(L_D1) { cout << "Send RESET signal to the Epiphany device..."; diag(L_D2) cout << endl; else cout << " "; }
//			SendReset(pEpiphany, E_RESET_ESYS);
//			SendReset(pEpiphany, E_RESET_CHIP);
			SendReset(pEpiphany, E_RESET_CORES);
			diag(L_D1) { cout << "Done." << endl; }
		}

		if (srecFile[0] != '\0') {
			diag(L_D1) { std::cout << "Loading SREC file " << srecFile << " ..." << endl; }

			if (parseAndSendSrecFile(srecFile, pEpiphany, broadcast, sverbose) == EPI_ERR) {
				std::cerr << "ERROR: Can't parse SREC file." << std::endl;
				e_close(pEpiphany);
				return EPI_ERR;
			}

			if (run_target) {
				for (unsigned coreNum=0; coreNum<fDiscoveredCoreIds.size(); coreNum++) {
					diag(L_D1) { cout << "Send ILAT SET signal to core ID 0x" << hex << fDiscoveredCoreIds[coreNum] << dec << "..."; diag(L_D2) cout << endl; else cout << " "; }
					SendILAT(pEpiphany, fDiscoveredCoreIds[coreNum]);
					diag(L_D1) { cout << "Done." << endl; }
				}
			}

			diag(L_D1) { std::cout << "Done." << endl; }
		}

		e_close(pEpiphany);
	} else {
		std::cerr << "ERROR: Can't connect to Epiphany." << std::endl;
		return EPI_ERR;
	}

	delete pEpiphany;

	return status;
}


