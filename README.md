DOBRE EMILIA ILIANA - 323CB

so_init_loader: 

    -implementarea consta in initializarea loaderului de executabile care creeaza 
    o rutina ce intercepteaza segmentation fault

    -pentru fiecare seg fault se apeleaza handlerul prin sigaction care retine 
    informatiile in newAct si retine informatiile despre handlerul default in 
    variabila defaultHandlr


so_execute:

    -parsarea din schelet initializeaza structura ce defineste executabilul, dar mai 
    trebuie in plus alocata memorie pentru campul data 

    -folosesc data din segment drept vector care tine cont daca
    pagina a fost mapata <=> data[numar_pagina] = 1


segv_handler:

    -aflu segmentul paginii, adresa de fault se afla intre adresa de inceput a 
    segmentului si memoria totala a acestuia


------------Tratare cazuri si mapari------------

CAZUL1:

    -segmentul nu este unul cunoscut deci apelez handlerul default
    

CAZUL2:

    -segmentul exista, dar este deja mapat complet

    -mapat <=> valoarea vectorului *data* a ultimii pagini mapate 
    
    -dimensiunea totala a paginilor momentan mapate ale unui segment e data de 
    diferenta dintre fault_adress care se afla undeva intr-un segment, si adresa de 
    unde incepe segmentul 


CAZUL3:

    -segmentul exista si trebuie mapata o anumita pagina, o voi mapa direct din fisier 
    avand fd si offsetul si numarul paginii la care s-a ajuns cu maparea

    -SUBCAZURI:

        ***depaseste file_size***
            -pagina trebuie mapata
            
            -verific daca pagina de mapat este pagina
            din zona ce depaseste file_size

            -pagina cautata aici este ultima pagina care 
            are o parte din bytes reprezentati de informatie din
            fisier, restul fiind in afara urmand sa fie zeroizati
            
            
        ***nu depaseste ***
            -doar mapare
            
    -MAPARE:

        -dimensiunea in bytes a paginilor deja mapate pana acum este nr_page*PAGE_SIZE

        -adresa de start pentru maparea in memorie a paginii curente se afla ca intr-un 
        vector, la adresa de dupa toate paginile deja mapate 
