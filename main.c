#include <stdio.h>
#include <stdlib.h>

#define FAKTOR_BAKETIRANJA 5
#define BROJ_BAKETA 9

typedef struct{
    int godina;
    int mesec;
    int dan;
    int sat;
    int minut;
} Datum;

typedef struct{
    int status;
    int evidencioniBrojUplate;
    int indentifikacioniBrojClana;
    Datum datum;
    char mesec[10]; //naziv meseca za koji se vrsi uplata
    int uplaceniIznos; //do 10000 din
} Uplata;

typedef struct Slog{
    Uplata podatak;
    struct Slog* sledeci;
}Slog;

typedef struct{
    FILE* file;
    char fileName[10];
    int E; //adresa prvog baketa sa slobodnom lokacijom
} Datafile;

typedef struct{
    Slog slogovi[FAKTOR_BAKETIRANJA];
    //int* u; //pokazivac na prvi slog iz skupa sinonima baketa
    Slog* u;
    int* b; //pokazivac na prvi prethodni baket sa slobodnom lokacijom
    int* n; //prvi naredni baket sa slobodnom lokacijom;
    int e; //broj slobodnih lokacija baketa;
} Baket;

Datafile activeDatafile;

int displayMenu();
void formirajDatoteku();
void napraviPrazanSlog(Slog* slog);
void izaberiAktivnuDatoteku();
void prikaziAktivnuDatoteku();
void prikaziCeluDatoteku();
void prikaziJedanBaket(Baket* baket);
void prikaziJedanSlog(Slog* slog);
void unosPodatakaUplate(Uplata* uplata);
int transformisiKljuc(int kljuc);
void unesiNoviSlog();

int main()
{
    int selectedOption = 0;
    while((selectedOption=displayMenu()) != 0){
        switch(selectedOption)
        {
            case 1:
                formirajDatoteku();
                break;
            case 2:
                izaberiAktivnuDatoteku();
                break;
            case 3:
                prikaziAktivnuDatoteku();
                break;
            case 4:
                unesiNoviSlog();
                break;
            case 6:
                prikaziCeluDatoteku();
                break;
            default:
                printf("Nijedna opcija nije izabrana. \n");
                break;

        }
    }


    return 0;
}

int displayMenu()
{
    int selection = 0;
    printf("\n");
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izaberi opciju: \n");
    printf("\t 1. Formiraj datoteku \n");
    printf("\t 2. Izaberi akrivnu datoteku \n");
    printf("\t 3. Prikazi naziv aktivne datoteke \n");
    printf("\t 4. Upisi novi slog u aktivnu datoteku \n");
    printf("\t 5. Trazi slog u aktivnoj datoteci \n");
    printf("\t 6. Prikazi sve slogove aktivne datoteke \n");
    printf("\t 7. Brisanje sloga iz aktivne datoteke \n");
    printf("\t 0. Exit \n");
    printf("------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("\n");
    printf("Vas izbor: ");
    scanf("%d", &selection);
    return selection;
}

void formirajDatoteku(){
    Datafile datafile;
    Baket baket;
    int i, j;
    Slog prazanSlog;

    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Formiranje nove datoteke. \n");
    printf("Unesite ime datoteke: ");
    fflush(stdin);

    scanf("%s", datafile.fileName);

    printf("Uneli ste: %s \n", datafile.fileName);
    printf("Otvaranje datoteke: %s \n", datafile.fileName);

    if((datafile.file=fopen(datafile.fileName, "wb+"))!=NULL){
        printf("\n");
        printf("Datoteka %s je uspesno kreirana!", datafile.fileName);
    }
    else{
        printf("\n");
        printf("Datoteka %s nije uspesno kreirana!", datafile.fileName);
    }

    printf("\n Postavljanje kreirane datoteke za aktivnu.");
    datafile.E = 0;

    activeDatafile.E = 0;
    activeDatafile.file = datafile.file;
    strcpy(activeDatafile.fileName, datafile.fileName);
    printf("\nAktivna datoteka: %s", activeDatafile.fileName);

    napraviPrazanSlog(&prazanSlog);
    //inicijalizacija praznog baketa
    for(i = 0; i<FAKTOR_BAKETIRANJA; ++i){
        baket.slogovi[i] = prazanSlog;
    }
    baket.e = FAKTOR_BAKETIRANJA;
    baket.u = NULL; //adresa lokacije prvog sloga iz skupa sinonima


    for(i = 0; i < BROJ_BAKETA; ++i){

        if(i==0){
            baket.b = 0; //prvi prethodni sa slobodnom lokacijom
            baket.n = i + 1; //prvi sledeci sa slobodnom lokacijom
        }else if(i > 0 && i < BROJ_BAKETA){
            baket.b = i - 1;
            baket.n = i + 1;
        }else if(i == FAKTOR_BAKETIRANJA){
            baket.b = i - 1;
            baket.n = i;
        }


        if(fseek(datafile.file, i*sizeof(Baket), SEEK_SET)!=0){
            printf("\n Doslo je do greske prilikom inicijalizacije prazne datoteke!");
            return 0;
        }
        if(fwrite(&baket, sizeof(Baket), 1, datafile.file)!=1){
            printf("\n Doslo je do greske prilikom inicijalizacije prazne datoteke!");
            return 0;
        }

    }
    fwrite(&datafile.E, sizeof(int*), 1, datafile.file);

    fclose(datafile.file);
}

void napraviPrazanSlog(Slog* slog){
    slog->podatak.evidencioniBrojUplate = 0;
    slog->podatak.indentifikacioniBrojClana = 0;
    slog->podatak.uplaceniIznos = 0;
    slog->podatak.datum.godina = 0;
    slog->podatak.datum.mesec = 0;
    slog->podatak.datum.dan = 0;
    slog->podatak.datum.sat = 0;
    slog->podatak.datum.minut = 0;
    slog->podatak.mesec[0] = '\0';
    slog->podatak.status = 1;

    slog->sledeci = NULL;
}

void izaberiAktivnuDatoteku(){
    char fName[15];
    FILE* datafilePok = NULL;
    int choice = 0;
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Izbor aktivne datoteke. \n");
    printf("\t Trenutno aktivna datoteka: %s \n", activeDatafile.fileName);
    printf("\t Unesite ime datoteke: ");
    fflush(stdin);
    scanf("%s", &fName);
    datafilePok = fopen(fName, "r+");
    if(datafilePok!=NULL){
        activeDatafile.file = datafilePok;
        strcpy(activeDatafile.fileName, fName);

        printf("\n Izabrali ste datoteku sa imenom: %s", fName);
    }
    else{
        printf("Datoteka sa imenom --%s-- ne postoji!", fName);
        printf("Da li zelite da pokusate ponovo? \n \t1.Da 0. Ne");
        scanf("%d", &choice);
        if(choice == 1){
            izaberiAktivnuDatoteku();
        }

    }
    fclose(datafilePok);
}

void prikaziAktivnuDatoteku(){
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Prikaz aktivne datoteke. \n");

    if(activeDatafile.file != NULL){
        printf("\n Aktivna datoteka: %s", activeDatafile.fileName);
    }
    else{
        printf("Trenutno nema aktivne datoteke!");
    }

}


void prikaziJedanBaket(Baket* baket){
    int j;
    fflush(stdout);

    for(j = 0; j<FAKTOR_BAKETIRANJA; ++j){
        printf("\n\t\t Slog: %d", j);

        prikaziJedanSlog(&baket->slogovi[j]);

    }
}

void prikaziJedanSlog(Slog* slog){
    printf("\n\t\t\tEvidencioni broj uplate: %d", slog->podatak.evidencioniBrojUplate);

    printf("\n\t\t\tIdentifikacioni broj clana: %d",slog->podatak.indentifikacioniBrojClana);

    printf("\n\t\t\tUplaceni iznos: %d",slog->podatak.uplaceniIznos);

    printf("\n\t\t\tDatum: %d.%d.%d Vreme: %d:%d", slog->podatak.datum.dan, slog->podatak.datum.mesec,slog->podatak.datum.godina, slog->podatak.datum.sat, slog->podatak.datum.minut);

    printf("\n\t\t\tMesec uplate: %s",slog->podatak.mesec);

    if(slog->podatak.status==1){
        printf("\n\t\t\tStatus sloga: SLOBODAN");
    }
    else{
        printf("\n\t\t\tStatus sloga: ZAUZET");
    }

    printf("\n\t\t\tSledeci sinonim: %d", slog->sledeci);
}

void prikaziCeluDatoteku(){
    int i;
    FILE* datafilePok = NULL;
    Baket baket;
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Prikaz svih slogova aktivne datoteke. \n");

    datafilePok = fopen(activeDatafile.fileName, "r");
    if(datafilePok != NULL){
        rewind(datafilePok);
        for(i = 0; i < BROJ_BAKETA; ++i){

            if(fseek(datafilePok, i*sizeof(Baket), SEEK_SET)!=0){
                printf("Greska prilikom iteriranja kroz datoteku!");
                return;
            }

            if(fread(&baket, sizeof(Baket), 1, datafilePok)!=1){
                printf("Greska prilikom citanja iz datoteke!");
                return;
            }
            fflush(stdout);
            printf("\n\t Baket: %d", i);
            prikaziJedanBaket(&baket);
        }
    }
    else{
        printf("\n\n Nema aktivne datoteke.");
        return;
    }


}

void unosPodatakaUplate(Uplata* uplata){
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("\n\tUnos nove uplate: ");


    printf("\n\t\tUnesite identifikacioni broj clana: ");
    do{
        fflush(stdin);
        scanf("%d", &uplata->indentifikacioniBrojClana);
    }while(uplata->indentifikacioniBrojClana < 100000 && uplata->indentifikacioniBrojClana > 999999);

    printf("\n\t\tUnesite datum u obliku DAN MESEC GODINA: ");
    fflush(stdin);
    scanf("%d%d%d", &uplata->datum.dan, &uplata->datum.mesec, &uplata->datum.godina);

    printf("\n\t\tUnesite vreme u obliku SAT MINUT: ");
    fflush(stdin);
    scanf("%d%d", &uplata->datum.sat, &uplata->datum.minut);

    printf("\n\t\tUnesite naziv meseca za koji se vrsi uplata: ");
    do{
        fflush(stdin);
        scanf("%s", &uplata->mesec);
    }while(strlen(uplata->mesec)>10);

    printf("\n\t\tUnesite iznos uplate: ");
    do{
        fflush(stdin);
        scanf("%d", &uplata->uplaceniIznos);
    }while(uplata->uplaceniIznos < 0 && uplata->uplaceniIznos >= 10000);
}

int transformisiKljuc(int kljuc){
    return kljuc%BROJ_BAKETA + 1;
}

void unesiNoviSlog(){
    int i,j;
    Uplata uplata;
    Slog slog;
    Baket baket;
    FILE* datotekaZaUpis = NULL;
    int kljuc;
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Unos novog sloga. \n");

    if(activeDatafile.file == NULL){
        printf("\n Nema aktivne datoteke. Upis ne moze da se izvrsi!");
        return;
    }

    if((datotekaZaUpis = fopen(activeDatafile.fileName, "rb+")) == NULL){
        printf("Greska!");
        return;
    }

    printf("\n\t\tUnesite identifikacioni broj uplate: ");
    do{
        fflush(stdin);
        scanf("%d", &uplata.evidencioniBrojUplate);
        kljuc = uplata.evidencioniBrojUplate;
    }while(uplata.evidencioniBrojUplate < 1000000 && uplata.evidencioniBrojUplate > 9999999);

    kljuc = transformisiKljuc(kljuc);

    fseek(datotekaZaUpis, (kljuc-1)*sizeof(Baket), SEEK_SET);
    fread(&baket, sizeof(Baket), 1, datotekaZaUpis);

    //Da li vec postoji taj evidencioni broj?
    for(i = 0; i<FAKTOR_BAKETIRANJA; ++i){
        if(baket.slogovi[i].podatak.evidencioniBrojUplate == uplata.evidencioniBrojUplate){
            printf("\n\t Uplata sa tim evidencionim brojem vec postoji!");
            return;
        }
    }

    if(baket.e > 0){
        unosPodatakaUplate(&uplata);
        slog.podatak = uplata;
        slog.sledeci = NULL;

        for(j = 0; j<FAKTOR_BAKETIRANJA; ++j){
            if(baket.slogovi[j].podatak.status == 1){
                slog.podatak.status = 0;
                baket.slogovi[j] = slog;



                break;
            }
        }
        fseek(datotekaZaUpis, (kljuc-1)*sizeof(Baket), SEEK_SET);
        fwrite(&baket, sizeof(Baket), 1, datotekaZaUpis);
    }else{


    }
    fclose(datotekaZaUpis);
}
