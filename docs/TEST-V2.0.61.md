## V2.0.61 | Plop bij de knopverlichting

De vorige versie startte de codec vóór de LED-golf. PC6 bleef laag, maar dat garandeert geen isolatie van de jack tegen voedings- of massastoring. Nu volgt codec-initialisatie pas nadat de LED-golf klaar is en de LED-status 400 ms stabiel is. De animatiepulsen van de knoppen gebruiken maximaal de middenstand. De gewone knopfeedback blijft volledig beschikbaar en oude middenstandbits worden gewist zodra de animatie eindigt.

Dit is een softwarecorrectie voor de hoorbare plop rond de opstartverlichting. Op een fysieke FM1 moet nog worden vastgesteld of de jack stil blijft. Vergelijk bij gelijke interfacegain drie koude starts met V2.0.60, V2.0.61 en SHRUB. Noteer of een eventuele plop samenvalt met de knopverlichting of pas daarna klinkt.

