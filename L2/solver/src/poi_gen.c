/*
 * POI_GEN
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "struct_ext.h"
#include "pcg_ext.h"
#include "mc.h"
#include "cm.h"
#include "rcm.h"
#include "cmrcm.h"
#include "poi_gen.h"
#include "allocate.h"

extern int
POI_GEN(void)
{
	int nn;
	int ic0, icN1, icN2, icN3, icN4, icN5, icN6;
	int i, j, k, ib, ic, ip, icel, icou, icol, icouG;
	int ii, jj, kk, nn1, num, nr, j0, j1;
	double coef, VOL0, S1t, E1t;
        int isL, ieL, isU, ieU;


	NL = 6;
	NU = 6;

	BFORCE = (double *)allocate_vector(sizeof(double),ICELTOT);
	D      = (double *)allocate_vector(sizeof(double),ICELTOT);
	PHI    = (double *)allocate_vector(sizeof(double),ICELTOT);
        INL = (int *)allocate_vector(sizeof(int),ICELTOT);
        INU = (int *)allocate_vector(sizeof(int),ICELTOT);
        IAL = (int **)allocate_matrix(sizeof(int),ICELTOT,NL);
        IAU = (int **)allocate_matrix(sizeof(int),ICELTOT,NU);

        indexL = (int *)allocate_vector(sizeof(int),ICELTOT+1);
        indexU = (int *)allocate_vector(sizeof(int),ICELTOT+1);

        for (i = 0; i <ICELTOT ; i++) {
		BFORCE[i]=0.0;
		D[i]  =0.0;
		PHI[i]=0.0;
        	INL[i] = 0;
        	INU[i] = 0;
        	for(j=0;j<6;j++){
			IAL[i][j]=0;
			IAU[i][j]=0;
		}
		
        }
        for (i = 0; i <=ICELTOT ; i++) {
		indexL[i] = 0;
		indexU[i] = 0;
	}


/*********************************
 * INTERIOR & NEUMANN boundary's *
 *********************************/

	for(icel=0; icel<ICELTOT; icel++) {
		icN1 = NEIBcell[icel][0];
		icN2 = NEIBcell[icel][1];
		icN3 = NEIBcell[icel][2];
		icN4 = NEIBcell[icel][3];
		icN5 = NEIBcell[icel][4];
		icN6 = NEIBcell[icel][5];

		if(icN5 != 0) {
			icou = INL[icel] + 1;
			IAL[icel][icou-1] = icN5;
			INL[icel]         = icou;
		}

		if(icN3 != 0) {
			icou = INL[icel] + 1;
			IAL[icel][icou-1] = icN3;
			INL[icel]         = icou;
		}

		if(icN1 != 0) {
			icou = INL[icel] + 1;
			IAL[icel][icou-1] = icN1;
			INL[icel]         = icou;
		}

		if(icN2 != 0) {
			icou = INU[icel] + 1;
			IAU[icel][icou-1] = icN2;
			INU[icel]         = icou;
		}

		if(icN4 != 0) {
			icou = INU[icel] + 1;
			IAU[icel][icou-1] = icN4;
			INU[icel]         = icou;
		}

		if(icN6 != 0) {
			icou = INU[icel] + 1;
			IAU[icel][icou-1] = icN6;
			INU[icel]         = icou;
		}
	}

/*****************
 * MULTICOLORING *
 *****************/

	OLDtoNEW = (int *) allocate_vector(sizeof(int),ICELTOT);
	NEWtoOLD = (int *) allocate_vector(sizeof(int),ICELTOT);
	COLORindex = (int *) allocate_vector(sizeof(int),ICELTOT+1);

N111:
	fprintf(stderr, "\n\nYou have%8d elements\n", ICELTOT);
	fprintf(stderr, "How many colors do you need ?\n");
	fprintf(stderr, "  #COLOR must be more than 2 and\n");
	fprintf(stderr, "  #COLOR must not be more than%8d\n", ICELTOT);
	fprintf(stderr, "   CM   if #COLOR .eq. 0\n");
	fprintf(stderr, "  RCM   if #COLOR .eq. -1\n");
	fprintf(stderr, "  CMRCM if #COLOR .lt. -1\n");
	fprintf(stderr, "=>\n");
	fscanf(stdin, "%d", &NCOLORtot);
	if(NCOLORtot == 1 || NCOLORtot > ICELTOT) goto N111;

	if(NCOLORtot > 0) {
		MC(ICELTOT, NL, NU, INL, IAL, INU, IAU,
				&NCOLORtot, COLORindex, NEWtoOLD, OLDtoNEW);
		fprintf(stderr, "\n###  MultiColoring\n");
	} else if(NCOLORtot == 0) {
		CM(ICELTOT, NL, NU, INL, IAL, INU, IAU,
				&NCOLORtot, COLORindex, NEWtoOLD, OLDtoNEW);
		fprintf(stderr, "\n###  CM\n");
	} else if(NCOLORtot ==-1) {
		RCM(ICELTOT, NL, NU, INL, IAL, INU, IAU,
				&NCOLORtot, COLORindex, NEWtoOLD, OLDtoNEW);
		fprintf(stderr, "\n###  RCM\n");
	} else if(NCOLORtot < -1) {
		CMRCM(ICELTOT, NL, NU, INL, IAL, INU, IAU,
				&NCOLORtot, COLORindex, NEWtoOLD, OLDtoNEW);
		fprintf(stderr, "\n###  CMRCM\n");
	}

	fprintf(stderr, "\n### FINAL COLOR NUMBER%8d\n\n", NCOLORtot);

/********************************************
* 1D ordering: indexL, indexU, itemL, itemU *
*********************************************/


        for(i=0; i<ICELTOT; i++){
                indexL[i+1]=indexL[i]+INL[i];
                indexU[i+1]=indexU[i]+INU[i];
        }
        NPL = indexL[ICELTOT];
        NPU = indexU[ICELTOT];

        itemL = (int *)allocate_vector(sizeof(int),NPL);
        itemU = (int *)allocate_vector(sizeof(int),NPU);
        AL    = (double *)allocate_vector(sizeof(double),NPL);
        AU    = (double *)allocate_vector(sizeof(double),NPU);

        for(i=0; i<ICELTOT; i++){
                for(k=0;k<INL[i];k++){
                        kk=k+indexL[i];
                        itemL[kk]=IAL[i][k];
                }
                for(k=0;k<INU[i];k++){
                        kk=k+indexU[i];
                        itemU[kk]=IAU[i][k];
                }
        }

        free(INL);
        free(INU);
        free(IAL);
        free(IAU);



/*************************************
 * INTERIOR & NEUMANN BOUNDARY CELLs *
 *************************************/
	for(icel=0; icel<ICELTOT; icel++) {
			ic0  = NEWtoOLD[icel];

			icN1 = NEIBcell[ic0-1][0];
			icN2 = NEIBcell[ic0-1][1];
			icN3 = NEIBcell[ic0-1][2];
			icN4 = NEIBcell[ic0-1][3];
			icN5 = NEIBcell[ic0-1][4];
			icN6 = NEIBcell[ic0-1][5];

	                VOL0 = VOLCEL[icel];

       		        isL =indexL[icel  ];
       		        ieL =indexL[icel+1];
                	isU =indexU[icel  ];
                	ieU =indexU[icel+1];

			if(icN5 != 0) {
				icN5 = OLDtoNEW[icN5-1];
				coef = RDZ * ZAREA;
				D[icel] -= coef;

				if(icN5-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN5) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN5) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			if(icN3 != 0) {
				icN3 = OLDtoNEW[icN3-1];
				coef = RDZ * YAREA;
				D[icel] -= coef;

				if(icN3-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN3) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN3) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			if(icN1 != 0) {
				icN1 = OLDtoNEW[icN1-1];
				coef = RDZ * XAREA;
				D[icel] -= coef;

				if(icN1-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN1) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN1) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			if(icN2 != 0) {
				icN2 = OLDtoNEW[icN2-1];
				coef = RDZ * XAREA;
				D[icel] -= coef;

				if(icN2-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN2) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN2) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			if(icN4 != 0) {
				icN4 = OLDtoNEW[icN4-1];
				coef = RDZ * YAREA;
				D[icel] -= coef;

				if(icN4-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN4) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN4) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			if(icN6 != 0) {
				icN6 = OLDtoNEW[icN6-1];
				coef = RDZ * ZAREA;
				D[icel] -= coef;

				if(icN6-1 < icel) {
					for(j=isL; j<ieL; j++) {
						if(itemL[j] == icN6) {
							AL[j] = coef;
							break;
						}
					}
				} else {
					for(j=isU; j<ieU; j++) {
						if(itemU[j] == icN6) {
							AU[j] = coef;
							break;
						}
					}
				}
			}

			ii = XYZ[ic0-1][0];
			jj = XYZ[ic0-1][1];
			kk = XYZ[ic0-1][2];

			BFORCE[icel] = - (double)(ii + jj + kk) * VOLCEL[ic0-1];
		}


/****************************
 * DIRICHLET BOUNDARY CELLs *
 ****************************/
/* TOP SURFACE */
	for(ib=0; ib<ZmaxCELtot; ib++) {
		ic0  = ZmaxCEL[ib];
		coef = 2.0 * RDZ * ZAREA;
		icel = OLDtoNEW[ic0-1];
		D[icel-1] -= coef;
	}


	return 0;
}
