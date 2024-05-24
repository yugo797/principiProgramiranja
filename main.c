#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//osnovne komponente
int RAM[4096][16]; //RAM
int PC[12]; //program counter
int I; //indirektni ciklus
int OPR[3]; //operation registar
int MAR[12]; //memory address registar
int MBR[16]; //mem buffer
int AC[16]; //akumulator
int E; //prekoracenje

//taktovi
int S; //  1 ili 0, da li je recunar upaljen
int SC[2]; //sequence counter, koji je takt

//ciklusi unutar taktova
int F;
int R;

void Dekoder24(int SC[2], int S, int t[4]) { //postavlja taktove
    if (SC[0]==0 && SC[1]==0 && S==1) {
        t[0]=1;
		t[1]=0;
		t[2]=0;
		t[3]=0;
	}
    if (SC[0]==0 && SC[1]==1 && S==1) {
        t[0]=0;
		t[1]=1;
		t[2]=0;
		t[3]=0;
	}
    if (SC[0]==1 && SC[1]==0 && S==1){
        t[0]=0;
		t[1]=0;
		t[2]=1;
		t[3]=0;
	}
    if (SC[0]==1 && SC[1]==1 && S==1){
        t[0]=0;
		t[1]=0;
		t[2]=0;
		t[3]=1;
	}
}
void Dekoder38 (int OPR[3], int S, int q[8]) {  //dekoder stanja;  u zavisnosti od aktivnog q prva druga treca vrsta naredbi

    if (OPR[0]==0 && OPR[1]==0 && OPR[2]==0 && S==1) {
        q[0]=1;
		q[1]=0;
		q[2]=0;
		q[3]=0;
		q[4]=0;
		q[5]=0;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==0 && OPR[1]==0 && OPR[2]==1 && S==1){
        q[0]=0;
		q[1]=1;
		q[2]=0;
		q[3]=0;
		q[4]=0;
		q[5]=0;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==0 && OPR[1]==1 && OPR[2]==0 && S==1){
        q[0]=0;
		q[1]=0;
		q[2]=1;
		q[3]=0;
		q[4]=0;
		q[5]=0;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==0 && OPR[1]==1 && OPR[2]==1 && S==1){
        q[0]=0;
		q[1]=0;
		q[2]=0;
		q[3]=1;
		q[4]=0;
		q[5]=0;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==1 && OPR[1]==0 && OPR[2]==0 && S==1){
        q[0]=0;
		q[1]=0;
		q[2]=0;
		q[3]=0;
		q[4]=1;
		q[5]=0;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==1 && OPR[1]==0 && OPR[2]==1 && S==1){
        q[0]=0;
		q[1]=0;
		q[2]=0;
		q[3]=0;
		q[4]=0;
		q[5]=1;
		q[6]=0;
		q[7]=0;
	}
    if (OPR[0]==1 && OPR[1]==1 && OPR[2]==0 && S==1){
        q[0]=0;
		q[1]=0;
		q[2]=0;
		q[3]=0;
		q[4]=0;
		q[5]=0;
		q[6]=1;
		q[7]=0;
	}
    if (OPR[0]==1 && OPR[1]==1 && OPR[2]==1 && S==1){ //ako je q7==1, I==0 onda je naredba druge vrste + I==1 trece vrste
        q[0]=0;
		q[1]=0;
		q[2]=0;
		q[3]=0;
		q[4]=0;
		q[5]=0;
		q[6]=0;
		q[7]=1;
	}
}

int dekUBin (int broj, int duzinaN, int niz[]){ //dekadni broj u binarni

    for (int i = 0; i <duzinaN; i++) {
        niz[i]=0;
    }
    int b = broj;

    if (broj<0) {
        broj = -1*broj;
    }

    for (int i = duzinaN-1; i >=0; i--) {
        niz[i] = broj%2;
        broj = broj/2;
    }

    if (b < 0) {
        for (int i = 0; i < duzinaN; i++) {
            if (niz[i]==1) {
                niz[i] = 0;
            }
            else
                niz[i] = 1;
        }
        inkrementNoE(niz, duzinaN);
    }
        return niz;
}

void cuvaURAM(int a, int adr) { //cuva 12 bita zadate memorije u ram

    for (int i = 4; i<16; i++) {
        RAM[adr][i] = (a&(1<<11))!=0;
        a<<=1;
    }

}

int binUDek (int reg[], int n) { //binarni zapis u dekadni

    int suma = 0;
    int br = 0;
    for (int i = n-1; i>=0; i--) {

        if (reg[i] == 1) {
                    suma+=pow(2, br);
            }
            br++;
    }
    return suma;

}

void shiftR(int reg[], int n) { //desni shift

        int tmp = reg[n-1];

        for (int i = n-1; i >=0; i--) {
            reg[i] = reg[i-1];
        }

        reg[0] = E;
        E = tmp;

}

void shiftL(int reg[], int n) { //lijevi shift (x^2)

        int tmp = reg[0];

        for (int i = 0; i<n-1; i++) {
            reg[i] = reg[i+1];
        }

        reg[n-1] = E;
        E = tmp;

}

void inkrementNoE(int reg[], int n) {

    int q=1;
    for(int i=n-1;i>=0;i--) {
        int prenos = reg[i] & q;
        reg[i] = q ^ reg[i];
        q = prenos;
    }

}
void inkrement(int reg[], int n) { //inkrementacija vr. registra

    int q=1;
    for(int i=n-1;i>=0;i--) {
        int prenos = reg[i] & q; //prenos postoji ako su oboje 1
        reg[i] = q ^ reg[i];
        q = prenos;
    }
    E=q;
}

void upaljenKompjuter () {

    S = 1;
    F = 0;
    R = 0;
    SC[0] = 1;
    SC[1] = 1;

	while (S == 1) {

		if (SC[0]==0 && SC[1]==0) {
			SC[0]=0;
			SC[1]=1;
		} else if (SC[0]==0 && SC[1]==1) {
			SC[0]=1;
			SC[1]=0;
		} else if (SC[0]==1 && SC[1]==0) {
			SC[0]=1;
			SC[1]=1;
		} else if(SC[0]==1 && SC[1]==1){
			SC[0]=0;
			SC[1]=0;
		}

		int t[4];
		Dekoder24(SC, S, t);

		int q[8];
		Dekoder38(OPR, S, q);

		if(F==0 && R==0){

            printf("Fetch ciklus:\n");
            fetch(t, q);
            stampaRegistara();
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");

		} else if (F==0 && R==1) {

		    printf("Indirekttni ciklus:\n");
            indirekt(t, q);
            stampaRegistara();
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");

		} else if (F==1 && R==0) {

		    printf("Izvrsni ciklus:\n");
            exec(t, q);
            stampaRegistara();
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		}

	}


}

void prepisivanjeBita(int n, int iz[], int u[]){ //prepisuje n bita iz reg IZ u reg U
    for(int i=0; i<n; i++){
        u[i]=iz[i];
    }
}
void fetch (int t[4], int q[8]) {

    if (t[0]==1) {
       prepisivanjeBita(12, PC, MAR);
       return;
    }
    else if (t[1]==1) {
        int adrRAM = binUDek(MAR, 12);
        prepisivanjeBita(16, RAM[adrRAM], MBR);
        inkrementNoE(PC, 12);
        return;
    }
    else if (t[2]==1) {
        I=MBR[0];
        prepisivanjeBita(3, MBR+1, OPR);
        return;
    }
    else{
        if (q[7]!=1 && I==1) {
            R=1;
        }
        else {
            F=1;
        }
        return;
    }

}

void indirekt (int t[4], int q[8]) {
    if (t[0]==1) {
        prepisivanjeBita(12, MBR+4, MAR);
        return;
    }
    else if (t[1]==1) {
        int adrRam = binUDek(MAR, 12);
        prepisivanjeBita(16, RAM[adrRam], MBR);
        return;
    }
    else if (t[2]==1) {
        return;
    }
    else {
        F=1;
        R=0;
        return;
        }
}

void exec (int t[4], int q[8]) {

    if(q[0]==1){ //AND; binarno & izmedju akumulatora i mem adrese

        if(t[0]== 1){
            prepisivanjeBita(12, MBR+4, MAR);
             return;

        } else if(t[1]==1){

            int adr = binUDek(MAR, 12);
            prepisivanjeBita(16, RAM[adr], MBR);
             return;

        } else if(t[2]==1){
            for (int i = 0; i <16; i++) {
            AC[i] = AC[i] & MBR[i];
             return;
                        }

        } else if(t[3]==1)
            F = 0;



    } else if(q[1]==1){ //ADD; sabiranje akumulatora sa mem adr

        if (t[0]==1) {
            prepisivanjeBita(12, MBR+4, MAR);
             return;

        } else if (t[1]==1) {
            int adr = binUDek(MAR, 12);
            prepisivanjeBita(16, RAM[adr], MBR);
             return;

        } else if (t[2]==1) {
            int Pren=0;
            for (int i = 15; i >=0; i--) {
				int q = AC[i];
                AC[i] = (AC[i]^MBR[i]) ^ Pren;
                Pren = ((q^MBR[i])&Pren) | (q&MBR[i]);
            }
            E=Pren;

        } else {
            F=0;
        }

    } else if(q[2]==1){ //LDA; sa mem adrese u akumulator


         if (t[0]==1) {
            prepisivanjeBita(12, MBR+4, MAR);
             return;

        } else if (t[1]==1) {

            int adrsa = binUDek(MAR, 12);
            prepisivanjeBita(16, RAM[adrsa], MBR);


        } else if (t[2]==1) {

             prepisivanjeBita(16, MBR, AC);

        } else {
            F=0;
        }


    } else if(q[3]==1){ //STA; iz akumulatora na adresu

        if (t[0]==1) { //iz mbra u MAR, u MBRu se nalazi dostupna adres
          prepisivanjeBita(12, MBR+4, MAR);
             return;

        } else if (t[1]==1) { //iz akum u mbr
            //acc to mbr,
            prepisivanjeBita(16, AC, MBR);

        } else if (t[2]==1) { //iz mara uzima adresu, pronalazi je u ramu i prebaca mbr u ram
            int adrsa = binUDek(MAR, 12);
            prepisivanjeBita(16, MBR, RAM[adrsa]); //mbr u ram
        } else {
            F=0;
        }


    } else if(q[4]==1){ //BUN

        if (t[0]==1) {
                prepisivanjeBita(12, MBR+4, MAR);
        } else if (t[1]==1) {
                prepisivanjeBita(12, MAR, PC);
        } else if (t[2]==1) {
            return;
        } else {
            F=0;
        }

    } else if(q[5]==1){ //BSA

        if (t[0]==1) {
            prepisivanjeBita(12, MBR+4, MAR);
        } else if (t[1]==1) {
            int adr = binUDek(MAR, 12);
            prepisivanjeBita(12, PC, RAM[adr]+4);
            return;

        } else if (t[2]==1) {
            prepisivanjeBita(12, MAR, PC);
            inkrementNoE(PC, 12);
            return;

        } else {
            F=0;
        }

    } else if(q[6]==1){ //ISZ, increment skip if 0

        if (t[0]==1) {
           prepisivanjeBita(12, MBR+4, MAR);

        } else if (t[1]==1) {

           int a = binUDek(MAR, 12);
           inkrementNoE(RAM[a], 16);

        } else if (t[2]==1) {
            int nula=0;
            int a = binUDek(MAR, 12);
            for (int i = 0; i<16; i++){
                if (RAM[a][i]==1){
                      nula++;
                }
            }
            if (nula==0) {
                inkrementNoE(PC, 12);
            }
        } else {
            F=0;
        }

    } else if(q[7] && I==0){
        int niz[12];

        prepisivanjeBita(12, MBR+4, niz);
        int aa = binUDek(niz, 12);
            if (aa==2048) { //CLA, clear accum

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                        for (int i =0; i < 16; i++) {
                            AC[i]=0;
                        }
                        F=0;
                }

            }else if (aa==1024) {//CLE, clear E

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    E=0;
                    F=0;
                }

            }else if (aa==512) {//CMA, complement acc

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    for (int i= 0; i < 16; i++) {
                            if (AC[i]==0){
                                AC[i]=1;
                            } else
                            AC[i]=0;
                        }
                         F=0;
                }

            }else if (aa==256) {//CME, complement E

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    if (E==1)
                          E=0;
                        else
                          E=1;
                     F=0;
                }

            }else if (aa==128) {//CIR, desni shift acc i E

                if(t[0]==1) {
                } else if (t[1]==1) {
                } else if (t[2]==1) {
                } else {

                    shiftR(AC, 16);
                   F=0;
                }

            }else if (aa==64) {//CIL
                if(t[0]==1) {
                } else if (t[1]==1) {
                } else if (t[2]==1) {
                } else {
                    shiftL(AC, 16);
                   F=0;
                }

            }else if (aa==32) {//INC, inkrem akumulator

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    inkrement(AC, 16);
                     F=0;
                }

            }else if (aa==16) {//SPA, skip positive ac

                if(t[0]==1) {
                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    if (AC[0]==0)
                            inkrementNoE(PC, 12);
                             F=0;
                }

            }else if (aa==8) {//SNA, skip negative ac
                if(t[0]==1) {

                } else if (t[1]==1) {
                } else if (t[2]==1) {
                } else {
                    if (AC[0]==1)
                            inkrementNoE(PC, 12);
                             F=0;
                }

            }else if (aa==4) {//SZA, skip zero ac

                if(t[0]==1) {

                } else if (t[1]==1) {
                } else if (t[2]==1) {
                } else {
                    int brojac=0;
                        for (int i = 0; i<16; i++) {
                            if (AC[i]==1)
                                brojac++;
                        }
                        if (brojac == 0){
                            inkrementNoE(PC, 12);
                        }
                         F=0;
                }

            }else if (aa==2) {//SZE, skip zero E
                if(t[0]==1) {
                } else if (t[1]==1) {
                } else if (t[2]==1) {
                } else {
                    if (E==0){
                            inkrementNoE(PC, 12);
                        }
                         F=0;
                }

            }else if (aa==1) {//HLT, halt
                if(t[0]==1) {

                } else if (t[1]==1) {
                    return;
                } else if (t[2]==1) {
                    return;
                } else {
                    S=0;
                    F=0;
                }

            }
    }
}
void prekid (int t[4], int q[8]) {
     return;
}

void openFile(char* imeFajla) {

    dekUBin(0, 12, PC);
	char* pctr;  //cita PC
	char* cmd; //cita komandu
	char* memLokacija; //cita zadatu mem lokaciju

	char linija[50];

	int pctrAtoi;
	int memLokacijaAtoi;

	FILE* f = fopen(imeFajla, "r");

	while(!feof(imeFajla)) {

		fgets(linija, 50, f);

		if (strcmp(linija, "\n")==0) {
			continue;
		}

		pctr = strtok(linija, " \n");
		cmd = strtok(NULL, " \n");
		memLokacija = strtok(NULL, " \n");

		if(strcmp(pctr, "#")==0) {
            return;
		}

		if (memLokacija != 	NULL) {

			pctrAtoi = atoi(pctr);
			memLokacijaAtoi = atoi(memLokacija);

			int x = binUDek(PC, 12);
			if (x==0)
                dekUBin(pctrAtoi, 12, PC);

			if (strcmp(cmd, "DEC")==0) {
				for (int i = 0; i < 16; i ++) {
					RAM[pctrAtoi][i] = (memLokacijaAtoi&(1<<15))!=0;
					memLokacijaAtoi<<=1;
				}
			}   else if (strstr(cmd, "AND")!=NULL) {

                    RAM[pctrAtoi][1] = 0;
                    RAM[pctrAtoi][2] = 0;
                    RAM[pctrAtoi][3] = 0;


            } else if (strstr(cmd, "ADD")!=NULL) {

                    RAM[pctrAtoi][1] = 0;
                    RAM[pctrAtoi][2] = 0;
                    RAM[pctrAtoi][3] = 1;


            }else if (strstr(cmd, "LDA")!=NULL) {

                    RAM[pctrAtoi][1] = 0;
                    RAM[pctrAtoi][2] = 1;
                    RAM[pctrAtoi][3] = 0;


            }else if (strstr(cmd, "STA")!=NULL) {

                    RAM[pctrAtoi][1] = 0;
                    RAM[pctrAtoi][2] = 1;
                    RAM[pctrAtoi][3] = 1;


            }else if (strstr(cmd, "BUN")!=NULL) {

                    RAM[pctrAtoi][1] = 1;
                    RAM[pctrAtoi][2] = 0;
                    RAM[pctrAtoi][3] = 0;


            }else if (strstr(cmd, "BSA")!=NULL) {

                    RAM[pctrAtoi][1] = 1;
                    RAM[pctrAtoi][2] = 0;
                    RAM[pctrAtoi][3] = 1;


            }else if (strstr(cmd, "ISZ")!=NULL) {

                    RAM[pctrAtoi][1] = 1;
                    RAM[pctrAtoi][2] = 1;
                    RAM[pctrAtoi][3] = 0;


            }



            if (strcmp(cmd, "DEC")!=0) {
                cuvaURAM(memLokacijaAtoi, pctrAtoi);

            }
            if (strstr(cmd, "*")!=NULL && strcmp(cmd, "DEC")!=0){

                    RAM[pctrAtoi][0] = 1;

            } else if (strstr(cmd, "*")==NULL && strcmp(cmd, "DEC")!=0){

                    RAM[pctrAtoi][0] = 0;

            }
                    }
            else {

                pctrAtoi = atoi(pctr);
                RAM[pctrAtoi][0] = 0;
                RAM[pctrAtoi][1] = 1;
                RAM[pctrAtoi][2] = 1;
                RAM[pctrAtoi][3] = 1;
            int x = binUDek(PC, 12);
            if(x==0){
                dekUBin(pctrAtoi, 12, PC);
            }

			if (strcmp(cmd, "CLA")==0) {
                cuvaURAM(2048, pctrAtoi);

            }else if (strcmp(cmd, "CLE")==0) {
                cuvaURAM(1024, pctrAtoi);

            }else if (strcmp(cmd, "CMA")==0) {
                cuvaURAM(512, pctrAtoi);

            }else if (strcmp(cmd, "CME")==0) {
                cuvaURAM(256, pctrAtoi);

            }else if (strcmp(cmd, "CIR")==0) {
                cuvaURAM(128, pctrAtoi);

            }else if (strcmp(cmd, "CIL")==0) {
                cuvaURAM(64, pctrAtoi);

            }else if (strcmp(cmd, "INC")==0) {
                cuvaURAM(32, pctrAtoi);

            }else if (strcmp(cmd, "SPA")==0) {
                cuvaURAM(16, pctrAtoi);

            }else if (strcmp(cmd, "SNA")==0) {
                cuvaURAM(8, pctrAtoi);

            }else if (strcmp(cmd, "SZA")==0) {
                cuvaURAM(4, pctrAtoi);

            }else if (strcmp(cmd, "SZE")==0) {
                cuvaURAM(2, pctrAtoi);

            }else if (strcmp(cmd, "HLT")==0) {
                cuvaURAM(1, pctrAtoi);

            }

		}

}



void stampaRegistara() {

    int br = 0;
    printf("PC: ");
    for (int i = 0; i <12; i++) {
        if(br%4==0){
            printf(" ");
        }
        printf("%d", PC[i]);
        br++;
    }

    br = 0;
    printf("\nMAR: ");
    for (int i = 0; i <12; i++) {
        if(br%4==0){
            printf(" ");
        }
        printf("%d", MAR[i]);
        br++;
    }

    br = 0;
    printf("\nMBR: ");
    for (int i = 0; i <16; i++) {
        if(br%4==0){
            printf(" ");
        }
        printf("%d", MBR[i]);
        br++;
    }

    br = 0;
    printf("\nAC: ");
    for (int i = 0; i <16; i++) {
        if(br%4==0){
            printf(" ");
        }
        printf("%d", AC[i]);
        br++;
    }
    br = 0;
    printf("\n");

}

void printZadato() {

    int a;
    printf("Unesite broj linija RAM-a koje zelite da stampate: ");
    scanf("%d", &a);

    int b[a];
    for (i = 0; i<a; i++) {
        printf("Unesite linije RAM-a koje zelite da stampate: ");
        scanf("%d", &b[i]);
    }
    int j;
    for (i = 0; i < a; i++) {
        printf("RAM[%d] - :", b[i]);
        for (j = 0; j<16; j++) {
            printf("%d", RAM[b[i]][j]);
        }

        printf("\n");

    }
}

int main() {

    openFile("test.txt");
    upaljenKompjuter();

    printZadato();

    return 0;
}
