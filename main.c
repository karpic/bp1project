#include <stdio.h>
#include <stdlib.h>

#define FAKTOR_BAKETIRANJA 5
#define BROJ_BAKETA 9

typedef struct{
    int brBaketa;
    int brSloga;

}Pokazivac;

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
    Pokazivac sledeci;
}Slog;

typedef struct{
    FILE* file;
    char fileName[10];
    int E; //adresa prvog baketa sa slobodnom lokacijom
} Datafile;

typedef struct{
    Slog slogovi[FAKTOR_BAKETIRANJA];
    //int* u; //pokazivac na prvi slog iz skupa sinonima baketa
    Pokazivac u;
    int b; //pokazivac na prvi prethodni baket sa slobodnom lokacijom
    int n; //prvi naredni baket sa slobodnom lokacijom;
    int e; //broj slobodnih lokacija baketa;
} Baket;

Datafile activeDatafile;
Pokazivac tempPrethodni;

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
int pronadjiSledeci(int rbBaketa);
int pronadjiPrethodni(int rbBaketa);
Pokazivac traziSlog(int evidencioniBroj, FILE* datafile);
void traziSlogUnosomEvBroja();
void updateSlogPok(Pokazivac stariPok, Pokazivac zaUnos, FILE* datafile);
void updateSlogPokBaket(Baket* baket, Pokazivac stariPok, Pokazivac zaUpis);

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
            case 5:
                traziSlogUnosomEvBroja();
                break;
            case 6:
                prikaziCeluDatoteku();
                break;
            case 7:
                obrisiSlog();
                break;
            case 9:
                pokazi(activeDatafile.file);
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
    Pokazivac nullPokazivac;

    nullPokazivac.brBaketa = -1;
    nullPokazivac.brSloga = -1;

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
    printf("\nE: %d", activeDatafile.E);

    napraviPrazanSlog(&prazanSlog);
    //inicijalizacija praznog baketa
    for(i = 0; i<FAKTOR_BAKETIRANJA; ++i){
        baket.slogovi[i] = prazanSlog;
    }
    baket.e = FAKTOR_BAKETIRANJA;
    baket.u = nullPokazivac; //adresa lokacije prvog sloga iz skupa sinonima


    for(i = 0; i < BROJ_BAKETA; ++i){

        if(i==0){
            baket.b = -1; //prvi prethodni sa slobodnom lokacijom, -1 zato sto je to prvi baket
            baket.n = i; //prvi sledeci sa slobodnom lokacijom
        }else if(i > 0 && i < BROJ_BAKETA){
            baket.b = i - 1;
            baket.n = i + 1;
        }else if(i == FAKTOR_BAKETIRANJA-1){
            baket.b = i;
            baket.n = -1;
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

    //zaglavlje datoteke, upiuje se relativna adresa prvog slobodnog baketa
    //rewind(datafile.file);

    //fseek(datafile.file, BROJ_BAKETA*sizeof(Baket), SEEK_SET);
    //fwrite(activeDatafile.E, sizeof(int*), 1, datafile.file);

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

    //ako je vrednost pokazivaca -1 nije inicijalizovano
    slog->sledeci.brBaketa = -1;
    slog->sledeci.brSloga = -1;
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
    datafilePok = fopen(fName, "rb+");
    if(datafilePok!=NULL){
        activeDatafile.file = datafilePok;
        strcpy(activeDatafile.fileName, fName);

        fseek(datafilePok, sizeof(int), SEEK_END);
        fread(&activeDatafile.E, sizeof(int), 1, activeDatafile.file);
        printf("\n\t Adresa prvog slobodnog baketa je: %d", activeDatafile.E);

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

    printf("\n\t\t\tSledeci sinonim: Baket %d Slog %d", slog->sledeci.brBaketa, slog->sledeci.brSloga);
}

void prikaziCeluDatoteku(){
    int i;
    FILE* datafilePok = NULL;
    Baket baket;
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("Izabrana opcija: Prikaz svih slogova aktivne datoteke. \n");

    datafilePok = fopen(activeDatafile.fileName, "rb+");
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
            printf("\n\t   Prvi prethodni sa sl. lokacijom: %d", baket.b);
            printf("\n\t   Prvi sledeci sa sl. lokacijom: %d", baket.n);

            prikaziJedanBaket(&baket);
        }
    }
    else{
        printf("\n\n Nema aktivne datoteke.");
        return;
    }
    //zatvaranje datoteke nakon izlistavanja celog sadrzaja
    fclose(datafilePok);
}

void unosPodatakaUplate(Uplata* uplata){
    printf("\n\n------------------------------------------------\n");
    printf("------------------------------------------------\n");
    printf("\n\tUnos nove uplate: ");



    do{
        printf("\n\t\tUnesite identifikacioni broj clana: ");
        fflush(stdin);
        scanf("%d", &uplata->indentifikacioniBrojClana);
    }while(uplata->indentifikacioniBrojClana < 100000 || uplata->indentifikacioniBrojClana > 999999);

    printf("\n\t\tUnesite datum u obliku DAN MESEC GODINA: ");
    fflush(stdin);
    scanf("%d%d%d", &uplata->datum.dan, &uplata->datum.mesec, &uplata->datum.godina);

    printf("\n\t\tUnesite vreme u obliku SAT MINUT: ");
    fflush(stdin);
    scanf("%d%d", &uplata->datum.sat, &uplata->datum.minut);


    do{
        printf("\n\t\tUnesite naziv meseca za koji se vrsi uplata: ");
        fflush(stdin);
        scanf("%s", &uplata->mesec);
    }while(strlen(uplata->mesec)>10);


    do{
        printf("\n\t\tUnesite iznos uplate: ");
        fflush(stdin);
        scanf("%d", &uplata->uplaceniIznos);
    }while(uplata->uplaceniIznos < 0 || uplata->uplaceniIznos >= 10000);
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
    Pokazivac prethodniSlog;
    Pokazivac trenutniSlog;
    int relAdresaBaketPrek;

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


    do{
        printf("\n\t\tUnesite identifikacioni broj uplate: ");
        fflush(stdin);
        scanf("%d", &uplata.evidencioniBrojUplate);
        kljuc = uplata.evidencioniBrojUplate;
    }while(uplata.evidencioniBrojUplate < 1000000 || uplata.evidencioniBrojUplate > 9999999);

    kljuc = transformisiKljuc(kljuc);

    fseek(datotekaZaUpis, (kljuc-1)*sizeof(Baket), SEEK_SET);
    fread(&baket, sizeof(Baket), 1, datotekaZaUpis);


    prethodniSlog = traziSlog(uplata.evidencioniBrojUplate, datotekaZaUpis);
    if(prethodniSlog.brBaketa != -1 && prethodniSlog.brSloga != -1){
        printf("\n\t Slog sa datim evidencionim brojem vec postoji! Baket %d, Slog %d", prethodniSlog.brBaketa, prethodniSlog.brSloga);
        return;
    }
    printf("\n\t Prethodni slog: baket %d slog %d", prethodniSlog.brBaketa, prethodniSlog.brSloga);

    if(baket.e > 0){
        unosPodatakaUplate(&uplata);
        slog.podatak = uplata;
        //slog.sledeci = tempPrethodni;



        for(j = 0; j<FAKTOR_BAKETIRANJA; ++j){
            if(baket.slogovi[j].podatak.status == 1){
                slog.podatak.status = 0;
                //baket.slogovi[j] = slog;

                if(baket.e == FAKTOR_BAKETIRANJA){
                    baket.u.brBaketa = kljuc - 1;
                    baket.u.brSloga = j;
                    slog.sledeci.brBaketa = -1;
                    slog.sledeci.brSloga = -1;
                }
                else{
                    trenutniSlog.brBaketa = kljuc - 1;
                    trenutniSlog.brSloga = j;
                    if(tempPrethodni.brBaketa != kljuc-1){
                        updateSlogPok(tempPrethodni, trenutniSlog, datotekaZaUpis);
                    }
                    else{
                        updateSlogPokBaket(&baket, tempPrethodni, trenutniSlog);
                    }
                    //slog.sledeci = tempPrethodni;
                    slog.sledeci.brBaketa = -1;
                    slog.sledeci.brSloga = -1;

                }
                baket.slogovi[j] = slog;

                baket.e = baket.e - 1;
                baket.n = pronadjiSledeci(kljuc-1);
                printf("\n\t Adresa prvog sledeceg baketa sa slogobnom lokacijom je: %d", baket.n);

                baket.b = pronadjiPrethodni(kljuc-1);
                printf("\n\t Adresa prvog prethodnog baketa sa slobodnom lokacijom je: %d", baket.b);
                break;
            }
        }
        fseek(datotekaZaUpis, (kljuc-1)*sizeof(Baket), SEEK_SET);
        fwrite(&baket, sizeof(Baket), 1, datotekaZaUpis);
    }else{
        //upisivanje sloga prekoracioca u baket cija je relativna adresa u indeksu E

        //ucitavanje baketa u koji se upiuje
        /*rewind(datotekaZaUpis);
        fseek(datotekaZaUpis, sizeof(int), SEEK_END);
        fread(&relAdresaBaketPrek, sizeof(int), 1, datotekaZaUpis);*/
        relAdresaBaketPrek = prviBaketSaSlobodnomLokacijom();
        printf("\n Relativna adresa prvog slobodnog: %d", relAdresaBaketPrek);
        rewind(datotekaZaUpis);
        fseek(datotekaZaUpis, relAdresaBaketPrek*sizeof(Baket), SEEK_SET);
        fread(&baket, sizeof(Baket), 1, datotekaZaUpis);

        //unos podataka
        unosPodatakaUplate(&uplata);
        slog.podatak = uplata;

        for(j = 0; j<FAKTOR_BAKETIRANJA; ++j){
            if(baket.slogovi[j].podatak.status == 1){
                slog.podatak.status = 0;

                trenutniSlog.brBaketa = relAdresaBaketPrek;
                trenutniSlog.brSloga = j;

                if(tempPrethodni.brBaketa != relAdresaBaketPrek){
                        updateSlogPok(tempPrethodni, trenutniSlog, datotekaZaUpis);
                }
                else{
                    updateSlogPokBaket(&baket, tempPrethodni, trenutniSlog);
                }
                //updateSlogPok(tempPrethodni, trenutniSlog, datotekaZaUpis);

                slog.sledeci.brBaketa = -1;
                slog.sledeci.brSloga = -1;

                baket.slogovi[j] = slog;

                baket.e = baket.e - 1;
                baket.n = pronadjiSledeci(relAdresaBaketPrek);
                printf("\n\t Adresa prvog sledeceg baketa sa slogobnom lokacijom je: %d", baket.n);

                baket.b = pronadjiPrethodni(relAdresaBaketPrek);
                printf("\n\t Adresa prvog prethodnog baketa sa slobodnom lokacijom je: %d", baket.b);
                break;
            }
        }
        rewind(datotekaZaUpis);
        fseek(datotekaZaUpis, relAdresaBaketPrek*sizeof(Baket), SEEK_SET);
        fwrite(&baket, sizeof(Baket), 1, datotekaZaUpis);

    }

    //Upisivanje relativne adrese prvog baketa sa slobodnom lokacijom na kraj datoteke
    activeDatafile.E = prviBaketSaSlobodnomLokacijom();

    rewind(datotekaZaUpis);
    fseek(datotekaZaUpis, sizeof(int), SEEK_END);
    fwrite(&activeDatafile.E, sizeof(Baket), 1, datotekaZaUpis);


    fclose(datotekaZaUpis);
}

int pronadjiSledeci(int rbBaketa){
    Baket b;
    int i,j;
    int returnValue = -1;
    for(i = rbBaketa + 1; i < BROJ_BAKETA; i++){
        fseek(activeDatafile.file, i*sizeof(Baket), SEEK_SET);
        fread(&b, sizeof(Baket), 1, activeDatafile.file);

        for(j = 0; j < FAKTOR_BAKETIRANJA; ++j){
            if(b.slogovi[j].podatak.status == 1){
                returnValue = i;
                break;
            }
        }
        if(returnValue != -1){
            break;
        }
    }
    return returnValue;
}

int pronadjiPrethodni(int rbBaketa){
    Baket b;
    int i,j;
    int returnValue = -1;
    for(i = rbBaketa- 1; i >= 0; i--){
        fseek(activeDatafile.file, i*sizeof(Baket), SEEK_SET);
        fread(&b, sizeof(Baket), 1, activeDatafile.file);

        for(j = 0; j < FAKTOR_BAKETIRANJA; ++j){
            if(b.slogovi[j].podatak.status == 1){
                returnValue = i;
                break;
            }
        }
        if(returnValue != -1){
            break;
        }
    }
    return returnValue;
}

int prviBaketSaSlobodnomLokacijom(){
    int i;
    int retVal;
    Baket b;
    for(i = 0; i < BROJ_BAKETA; i++){
        rewind(activeDatafile.file);
        fseek(activeDatafile.file, i*sizeof(Baket), SEEK_SET);
        fread(&b, sizeof(Baket), 1, activeDatafile.file);

        if(b.e > 0){
            retVal = i;
            break;
        }
    }

    return retVal;
}


Pokazivac traziSlog(int evidencioniBroj, FILE* datafile){
    Pokazivac startingPokazivac;
    Pokazivac returnVal;
    int relAdresaBaketa;
    Baket baket;


    relAdresaBaketa = transformisiKljuc(evidencioniBroj);

    rewind(datafile);
    fseek(datafile, (relAdresaBaketa-1)*sizeof(Baket), SEEK_SET);
    fread(&baket, sizeof(Baket), 1, datafile);

    startingPokazivac.brBaketa = baket.u.brBaketa;
    startingPokazivac.brSloga = baket.u.brSloga;

    if(startingPokazivac.brBaketa != -1 && startingPokazivac.brSloga != -1){
        while(startingPokazivac.brBaketa != -1){
            rewind(datafile);
            fseek(datafile, startingPokazivac.brBaketa*sizeof(Baket), SEEK_SET);
            fread(&baket, sizeof(Baket), 1, datafile);

            if(baket.slogovi[startingPokazivac.brSloga].podatak.evidencioniBrojUplate == evidencioniBroj){
                printf("\n\t Trazenje je uspesno! Broj baketa pronadjenog sloga: %d, redni broj sloga u baketu: %d", startingPokazivac.brBaketa, startingPokazivac.brSloga);
                returnVal.brBaketa = startingPokazivac.brBaketa;
                returnVal.brSloga = startingPokazivac.brSloga;

                return returnVal;
            }
            else{
                tempPrethodni.brBaketa = startingPokazivac.brBaketa;
                tempPrethodni.brSloga = startingPokazivac.brSloga;

                int tempB = baket.slogovi[startingPokazivac.brSloga].sledeci.brBaketa;
                int tempS = baket.slogovi[startingPokazivac.brSloga].sledeci.brSloga;

                //startingPokazivac.brBaketa = baket.slogovi[startingPokazivac.brSloga].sledeci.brBaketa;
                //startingPokazivac.brSloga = baket.slogovi[startingPokazivac.brSloga].sledeci.brSloga;
                startingPokazivac.brBaketa = tempB;
                startingPokazivac.brSloga = tempS;

            }

        }
    }
    else{
        printf("\n\t Ne postoji slog sa datim evidencionim brojem: %d", evidencioniBroj);
        //ako je prvi iz skupa sinonima, ako se na prvom mestu u baketu ne nalazi nista
        returnVal.brBaketa = -1;
        returnVal.brSloga = -1;

        return returnVal;
    }

    return startingPokazivac;
}

void traziSlogUnosomEvBroja(){
    FILE* datotekaZaTrazenje;
    Pokazivac slogPok;
    Slog slog;
    int evidencioniBroj;
    Baket baket;

    if(activeDatafile.file == NULL){
        printf("\n\t Nema aktivne datoteke!");
        return;
    }

    if((datotekaZaTrazenje = fopen(activeDatafile.fileName, "rb+")) == NULL){
        printf("Greska!");
        return;
    }


    do{
        printf("\n\t\tUnesite identifikacioni broj uplate: ");
        fflush(stdin);
        scanf("%d", &evidencioniBroj);

    }while(evidencioniBroj < 1000000 || evidencioniBroj > 9999999);


    slogPok = traziSlog(evidencioniBroj, datotekaZaTrazenje);
    if(slogPok.brBaketa != -1 && slogPok.brSloga != -1){
        //printf("\n\t Trazenje je uspesno!");
        //printf("\n\t Baket: %d, redni broj sloga u baketu: %d", slogPok.brBaketa, slogPok.brSloga);
        fseek(datotekaZaTrazenje, slogPok.brBaketa*sizeof(Baket), SEEK_SET);
        fread(&baket, sizeof(Baket), 1, datotekaZaTrazenje);
        prikaziJedanSlog(&baket.slogovi[slogPok.brSloga]);
    }
    else{
        printf("\n\t Trazenje nije uspesno!");
    }
    fclose(datotekaZaTrazenje);
}

void updateSlogPok(Pokazivac stariPok, Pokazivac zaUnos, FILE* datafile){
    Baket baket;

    if(datafile == NULL){
        printf("\t\n Doslo je do greske prilikom update-ovanje pokazivaca sloga");
        return;
    }
    rewind(datafile);
    fseek(datafile, stariPok.brBaketa*sizeof(Baket), SEEK_SET);
    fread(&baket, sizeof(Baket), 1, datafile);

    baket.slogovi[stariPok.brSloga].sledeci.brBaketa = zaUnos.brBaketa;
    baket.slogovi[stariPok.brSloga].sledeci.brSloga = zaUnos.brSloga;

    rewind(datafile);
    fseek(datafile, stariPok.brBaketa*sizeof(Baket), SEEK_SET);
    fwrite(&baket, sizeof(Baket), 1, datafile);

}

void updateSlogPokBaket(Baket* baket, Pokazivac stariPok, Pokazivac zaUpis){
    baket->slogovi[stariPok.brSloga].sledeci.brBaketa = zaUpis.brBaketa;
    baket->slogovi[stariPok.brSloga].sledeci.brSloga = zaUpis.brSloga;
}

void pokazi(FILE* datafile){
    int zaPrikaz;
    if(datafile == NULL){
        fopen(datafile, "rb+");
    }

    fseek(datafile, sizeof(int), SEEK_END);
    fread(&zaPrikaz, sizeof(int), 1, datafile);
    printf("\n\t Adresa prvog slobodnog baketa je: %d", zaPrikaz);
}

void obrisiSlog(){
    FILE* datotekaZaTrazenje;
    Pokazivac slogPok;
    Slog slog;
    int evidencioniBroj;
    Baket baket;
    Slog prazanSlog;

    napraviPrazanSlog(&slog);

    if(activeDatafile.file == NULL){
        printf("\n\t Nema aktivne datoteke!");
        return;
    }

    if((datotekaZaTrazenje = fopen(activeDatafile.fileName, "rb+")) == NULL){
        printf("Greska!");
        return;
    }


    do{
        printf("\n\t\tUnesite identifikacioni broj uplate: ");
        fflush(stdin);
        scanf("%d", &evidencioniBroj);

    }while(evidencioniBroj < 1000000 || evidencioniBroj > 9999999);


    slogPok = traziSlog(evidencioniBroj, datotekaZaTrazenje);
    if(slogPok.brBaketa != -1 && slogPok.brSloga != -1){
        //printf("\n\t Trazenje je uspesno!");
        //printf("\n\t Baket: %d, redni broj sloga u baketu: %d", slogPok.brBaketa, slogPok.brSloga);
        fseek(datotekaZaTrazenje, slogPok.brBaketa*sizeof(Baket), SEEK_SET);
        fread(&baket, sizeof(Baket), 1, datotekaZaTrazenje);

        baket.slogovi[slogPok.brSloga] = slog;

        fseek(datotekaZaTrazenje, slogPok.brBaketa*sizeof(Baket), SEEK_SET);
        fwrite(&baket, sizeof(Baket), 1, datotekaZaTrazenje);
    }
    else{
        printf("\n\t Uplata sa evidencionim brojem: %d ne postoji", evidencioniBroj);
    }
    fclose(datotekaZaTrazenje);
}


