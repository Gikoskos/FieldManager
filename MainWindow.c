/*
    MainWindow.c

    Copyright (C) 2018 George Koskeridis

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include "Common.h"
#include "MsgBoxError.h"
#include "GlobalData.h"
#include "Chart.h"
#include "Resources.h"
#include "Weather.h"
#include "Field.h"
#include "Name.h"
#include "Newsfeed.h"
#include <stdlib.h>
#include <stdio.h>
#include <windowsx.h>
#include <commctrl.h>
#include <richedit.h>
#include <mCtrl/chart.h>
#include <pthread.h>

#define HANDLE_WM_MOUSEHOVER HANDLE_WM_MOUSEMOVE
#define TID_POLLMOUSE 100
#define MOUSE_POLL_DELAY 500

#define MAX_LEN_DOUBLE_FIELD 18
#define MAX_FARMERS 2048
#define MAX_FARMERS_WSTR L"2048"
#define CROPLIST_MAXOFFS 10

ULONG total_farmers = 0, farmer_array_len = DEFAULT_ALLOC_SIZE;
int curr_farmer = -1;

char char_buff[_CVTBUFSIZE];
wchar_t wchar_buff[_CVTBUFSIZE];

wchar_t *crop_names[] = {
    L"καπνός",
    L"βαμβάκι",
    L"σιτάρι",
    L"βίκος",
    L"παντζάρι",
    L"ρύζι",
    L"φακή",
    L"μύρτιλο",
    L"βασιλικός",
    L"ιπποφαές",
    L"αλόη",
    L"ρόδι",
    L"αρώνια",
    L"δενδρολίβανο",
    L"κρανιά",
    L"στέβια",
    L"γκότζι μπέρι"
};

wchar_t *crop_description[] = {
    L"Ο καπνός είναι μονοετές, ποώδες φυτό και ανήκει στο γένος "
    L"Νικοτιανή (Nicotiana), το οποίο καλλιεργείται για τα φύλλα του, "
    L"τα οποία μετά από κατάλληλη επεξεργασία χρησιμοποιούνται για την "
    L"παραγωγή καπνικών προϊόντων, όπως τσιγάρα κλπ.",

    L"Το βαμβάκι είναι Αγγειόσπερμο, δικότυλο φυτό το οποίο ανήκει στην "
    L"τάξη των Μαλαχωδών και στην οικογένεια των Μαλαχοειδών. Ιθαγενές "
    L"των τροπικών περιοχών της Ασίας και της Αφρικής είναι γνωστό από "
    L"τα πανάρχαια χρόνια και καλλιεργείται για τις ίνες του.",

    L"Το σιτάρι ή στάρι ή σίτος (Triticum spp), είναι ένα φυτό που καλλιεργείται "
    L"σε όλο τον κόσμο. Είναι το δεύτερο παγκοσμίως σε συγκομιδή δημητριακό, μετά "
    L"τον αραβόσιτο, με τρίτο το ρύζι. Ο καρπός του σίτου είναι μια βασική τροφή, "
    L"που χρησιμοποιείται στην παρασκευή αλευριού, ζωοτροφών και ως πρώτη ύλη στην "
    L"παρασκευή αλκοολούχων ποτών και καυσίμων. Ο φλοιός του μπορεί να αποσπαστεί από "
    L"τον καρπό και να αλεστεί, δίνοντας το λεγόμενο πίτουρο. Ο σίτος καλλιεργείται επίσης "
    L"για τη βοσκή των ζώων, καθώς και για το άχυρο, τον κορμό του φυτού, που χρησιμοποιείται "
    L"ως ζωοτροφή ή υλικό κατασκευών. Το σιτάρι, όπως και τα άλλα δημητριακά, η βρώμη, η σίκαλη, "
    L"το κριθάρι, περιέχουν μία πρωτεΐνη, τη γλουτένη, στην οποία πολλοί άνθρωποι είναι δυσανεκτικοί "
    L"(αλλεργικοί κατά κάποιο τρόπο), εκδηλώνοντας τη λεγόμενη κοιλιοκάκη, ένα είδος εντεροπάθειας.",

    L"Ο Βίκος (Vicia) είναι γένος που περιλαμβάνει περίπου 140 είδη ανθοφόρων φυτών, τα οποία "
    L"είναι κοινώς γνωστά ως λαθούρια. Ανήκει στη οικογένεια των Κυαμοειδών (Fabaceae). Τα είδη "
    L"είναι ιθαγενή στην Ευρώπη, τη Βόρεια Αμερική, τη Νότια Αμερική, την Ασία και την Αφρική. "
    L"Κάποια άλλα γένη της υποοικογένειάς τους των Ψυχανθών (Faboideae), φέρουν επίσης ονόματα "
    L"τα οποία περιέχουν το \"vetch\" (\"βίκο\"), για παράδειγμα, τα vetchlings (Lathyrus) ή των γαλακτο-λαθουριών "
    L"(Astragalus). Η κουκιά (Vicia faba), μερικές φορές χωρίζεται στο μονοτυπικό γένος Faba· παρόλο "
    L"που δεν χρησιμοποιείται συχνά σήμερα, είναι ιστορικής σημασίας στην ταξινόμηση των φυτών, "
    L"όπως το ομώνυμο της τάξης Fabales, το Fabaceae και τα Faboideae. Η φυλή Vicieae στην οποία "
    L"τοποθετούνται τα λαθούρια, ονομάζεται μετά από τη σημερινή ονομασία του γένους. Μεταξύ των "
    L"στενών εν ζωή συγγενών των λαθουριών, είναι οι φακές (Lens) και τα πραγματικά μπιζέλια (Pisum).",

    L"Το παντζάρι (Beta vulgaris), ή κοκκινογούλι, είναι φυτό της οικογένειας Αμαρανθοειδή, η οποία "
    L"παλιότερα ήταν γνωστή ως Χηνοποδιοειδή. Είναι μονοετές ή διετές φυτό του οποίου η ρίζα είναι "
    L"εδώδιμη. Υπάρχουν πολλές καλλιεργήσιμες ποικιλίες, όπως το σέσκουλο, μια φυλλώδης ποικιλία "
    L"παντζαριού, και το ζαχαρότευτλο, της οποίας η ρίζα έχει υψηλή συγκέντρωση σε σάκχαρα. Όλες "
    L"οι καλλιεργήσιμες ποικιλίες παντζαριού προέρχονται από το υποείδος B. v. vulgaris. Άλλα "
    L"υποείδη του παντζαριού είναι τα B. v. maritima, το οποίο βρίσκεται στη Μεσόγειο, στις "
    L"Ευρωπαϊκές ακτές του Ατλαντικού, από τη Μέση Ανατολή μέχρι την Ινδία, και το B. v. adanensis, "
    L"το οποίο βρίσκεται στη βόρεια ανατολική Μεσόγειο. Η εξημέρωση του φυτού θεωρείται ότι "
    L"έλαβε χώρα την 2η χιλιετία π.Χ. στην περιοχή της Μεσογείου.",

    L"Το ρύζι είναι μονοκοτυλήδονο φυτό της οικογένειας των Ποοειδών (Poaceae) ή Αγρωστωδών "
    L"(Gramineae), που απαντάται σε δύο είδη (Oryza sativa, Όρυζα η ήμερη και Oryza glaberrima, "
    L"Όρυζα η λειοτάτη) με καταγωγή από την τροπική και υποτροπική Νότια Ασία και την Αφρική. Το "
    L"ρύζι είναι ένα από τα βασικά διατροφικά είδη της ανθρωπότητας: τα δυο είδη του αποτελούν "
    L"το ένα πέμπτο των συνολικά καταναλισκόμενων θερμίδων παγκοσμίως. (Ο όρος \"άγριο ρύζι\" μπορεί "
    L"να αναφέρεται σε άγρια είδη Όρυζα (Oryza), αλλά κατά σύμβαση αναφέρεται σε είδη του συγγενούς "
    L"είδους Zizania, τόσο άγρια όσο και καλλιεργούμενα). Το ρύζι συνήθως αναπτύσσεται σε ύψος 1-1,8 "
    L"μέτρα, με μακριά λεία φύλλα 50-100 εκατοστά σε μήκος και 2-2,5 εκατοστά πλάτος. Τα μικρά του "
    L"άνθη βγαίνουν σε κλαδιά 30-50 εκατοστών. Ο σπόρος είναι κοκκώδης (caryopsis) με μήκος 5-12 "
    L"χιλιοστά και 2-3 χιλιοστά διατομή.",

    L"Η φακή, ή με την επιστημονική της ονομασία Φακός ο μαγειρικός (Lens culinaris), είναι "
    L"αγγειόσπερμο, δικότυλο φυτό, που ανήκει στην οικογένεια των Κυαμοειδών και στην τάξη "
    L"των Κυαμωδών. Καλλιεργείται δε για το μικρό ομώνυμο εδώδιμο σπόρο του, που είναι ένα "
    L"από τα σημαντικότερα όσπρια. Είναι ένα από τα πρώτα φυτά που ξεκίνησε να καλλιεργεί "
    L"συστηματικά ο άνθρωπος.",

    L"Το μύρτιλο ή αλλιώς μπλούμπερι, με την αγγλική του ονομασία βρίσκεται στη λίστα των "
    L"εναλλακτικών καλλιεργειών που δείχνουν καλή προσαρμοστικότητα στα ελληνικά εδάφη και "
    L"μπορεί να έχει καλές αποδόσεις. Η μέση απόδοσή του φτάνει τα 1.000 κιλά ανά στρέμμα, "
    L"ενώ σε ένα στρέμμα μπαίνουν 200 φυτά. Το φυτό φτάνει σε ύψος τα 1,50-1,80 μ., προτιμά "
    L"περιοχές με μεγάλη ηλιοφάνεια, εδάφη με πολύ χαμηλό pΗ. Ο καρπός του έχει μπλε χρώμα "
    L"και χρησιμοποιείται μεταξύ των άλλων για την παρασκευή μαρμελάδας, ενώ πωλείται και ως "
    L"αποξηραμένος. Το δενδρύλλιο δίνει παραγωγή από την πρώτη κιόλας χρονιά, ενώ μετά το τρίτο "
    L"έτος αρχίζει να δίνει 3-4 κιλά το δένδρο και σε πλήρη παραγωγή φτάνει τα 5 κιλά. Με τιμή "
    L"5 ευρώ/κιλό κατά μέσο όρο, το καθαρό εισόδημα ανέρχεται στα 2.500 ευρώ/στρέμμα.",

    L"Ευδοκιμεί σε χωράφια μέσης σύστασης, πλούσια, σε ό,τι αφορά το κάλιο ποτιστικά με καλή "
    L"αποστράγγιση. Ο βασιλικός είναι φυτό με μεγάλη βλαστητική ανάπτυξη γι' αυτό έχει συνέχεια "
    L"ανάγκη από λιπαντικά (κομπόστα) στοιχεία και υγρασία στο έδαφος. Ο βασιλικός θέλει πολύ "
    L"ήλιο, για να αποκτήσει το γνωστό, χαρακτηριστικό του άρωμα. Συλλέγεται λίγο πριν την "
    L"ανθοφορία ή ξεραίνεται ολόκληρο σε μέρος δροσερό και αεριζόμενο. Η ζήτηση του βασιλικού "
    L"από τη φαρμακευτική βιομηχανία είναι μεγάλη και τα έσοδα που δίνει στον παραγωγό σημαντικά. "
    L"Η απόδοση φθάνει τα 400 κιλά αποξηραμένα φύλλα με καθαρά έσοδα για τον παραγωγό περίπου 800 "
    L"ευρώ. Σε περίπτωση που πρόκειται για βιολογική καλλιέργεια, τα έσοδα διπλασιάζονται και "
    L"κυμαίνονται στα 1.500-2.000 ευρώ.",

    L"Εισόδημα που φθάνει τις 2.000 ευρώ ανά στρέμμα μπορεί να αποφέρει η καλλιέργεια του ιπποφαούς "
    L"με τη μέθοδο της συμβολαιακής γεωργίας που εξασφαλίζει τη διάθεση της παραγωγής. Σημαντικό "
    L"πλεονέκτημα της καλλιέργειας του ιπποφαούς που συγκαταλέγεται στην κατηγορία των λεγόμενων "
    L"υπερτροφών (super foods) είναι ότι -σύμφωνα με τους γνώστες- έχει ελάχιστες απαιτήσεις "
    L"καλλιεργητικής φροντίδας, προσαρμόζεται ακόμη και στις πιο ακραίες καιρικές συνθήκες ξηρασίας "
    L"και παγετού, ενώ μπορεί να φυτευτεί και σε άγονα εδάφη. Εχει κατά μέσο όρο απόδοση 6-7 κιλά "
    L"καρπό ανά δένδρο, κάτι που σημαίνει ότι φυτεύοντας κάποιος 150 θηλυκά δένδρα ανά στρέμμα "
    L"παίρνει 900-1.000 κιλά κατά μέσο όρο τον χρόνο. Με τιμή παραγωγού 3 ευρώ/κιλό, προκύπτουν "
    L"ακαθάριστα έσοδα 3.000?3.500 ευρώ τον χρόνο και αφαιρώντας τα έξοδα, απομένουν 2.000 ευρώ καθαρό εισόδημα.",

    L"Η αλόη θεωρείται το «βότανο της αθανασίας» και όχι άδικα, αν λάβουμε υπόψη τα σημαντικά συστατικά "
    L"που περιέχουν τα φύλλα της και τις θεραπευτικές της ιδιότητες. Παράλληλα, η καλλιέργεια της αλόης "
    L"είναι και μία μεγάλη επαγγελματική ευκαιρία για τους Έλληνες παραγωγούς, αφού το τζελ αλόης έχει "
    L"τεράστια ζήτηση, αλλά μικρή εγχώρια παραγωγή. Το κάθε φυτό μπορεί να δώσει τουλάχιστον 4 φύλλα από "
    L"τρεις ώς τέσσερις φορές τον χρόνο. Μετά τον 4ο χρόνο το στρέμμα μπορεί να δώσει ως και 13.000 φύλλα. "
    L"Το κάθε φύλλο έχει βάρος 400 με 800 γραμμάρια, που σημαίνει ότι το ένα στρέμμα αποδίδει 7.000 κιλά "
    L"φύλλων, τα οποία εμπεριέχουν περίπου 70%-80% gel.",

    L"Εναλλακτική πρόταση σε νέους και παλιούς αγρότες προσφέρει η συμβολαιακή καλλιέργεια της ροδιάς, "
    L"που φτάνει να αποδίδει εισόδημα της τάξης έως και 1.200 ευρώ ανά στρέμμα. Η ροδιά είναι ανθεκτικό "
    L"φυλλοβόλο δένδρο που σπάνια προσβάλλεται από παράσιτα και μπορεί να φτάσει μέχρι τα 3-4 μ. Οι "
    L"κόκκινοι χυμώδεις καρποί του ροδιού έχουν λίγες θερμίδες και είναι εξαιρετικά πλούσιοι σε σάκχαρα, "
    L"βιταμίνες A, B, C, καθώς και σε μέταλλα όπως κάλλιο και πολυφαινόλες. Σε αυτά οφείλει το ρόδι την "
    L"αντιοξειδωτική και αντικαρκινική δράση του. Οι αποδόσεις ανά στρέμμα κυμαίνονται σε 2,5-3 τόνους "
    L"εμπορεύσιμο ρόδι, ενώ το κόστος εγκατάστασης ανά στρέμμα ανέρχεται σε περίπου 540 ευρώ.",

    L"Ευχάριστες εκπλήξεις κρύβουν ακόμα οι εναλλακτικές καλλιέργειες στην Ελλάδα. Συγκεκριμένα, η "
    L"αρώνια, που δεν είναι πολύ γνωστή, έχει κατά κάποιο τρόπο «επισκιαστεί» από άλλες πιο γνωστές "
    L"καλλιέργειες. Η αρώνια είναι φαρμακευτικό φυτό και χάρη στη χημική σύσταση των καρπών της προσφέρει "
    L"αναρίθμητα οφέλη στον ανθρώπινο οργανισμό. Ο καρπός της περιέχει βιταμίνες Α, Β1, Β2, Β3, Β5, Β6, "
    L"Β9, C, E, K και συγκεντρώνει υψηλές ποσότητες φλαβονοειδών, που είναι πολύ σημαντικά για τις αντιοξειδωτικές "
    L"ιδιότητές τους. Η καλλιέργειά είναι εξαιρετικά αποδοτική. Δύο με τρία χρόνια μετά τη φύτευση, φτάνει "
    L"τα 1.300-1.800 κιλά στρεμματική απόδοση και αποδίδει εισόδημα περίπου 1.000 ευρώ το στρέμμα.",

    L"Πρώτης τάξεως ευκαιρία για κάθε άλλο παρά ευκαταφρόνητο εισόδημα συνιστά η καλλιέργεια του αρωματικού "
    L"φυτού, που τα τελευταία χρόνια εμφανίζει μεγάλα περιθώρια ανάπτυξης για πωλήσεις εντός και εκτός ελληνικών "
    L"συνόρων. Το δενδρολίβανο υπόσχεται πολύ καλές αποδόσεις και παρά το γεγονός ότι η καλλιέργεια του φυτού "
    L"στην Ελλάδα είχε ξεκινήσει εδώ και δεκαετίες, εξακολουθεί να θεωρείται ένα νέο σχετικά πεδίο δράσης για "
    L"τον ελλαδικό χώρο προσφέροντας μια εναλλακτική δυναμική ανάπτυξης στον πρωτογενή τομέα, ενώ δίνει ώθηση "
    L"και στον δευτερογενή τομέα της μεταποίησης. Αποτελεί ελκυστική καλλιέργεια χάρη στο χαμηλό κόστος εγκατάστασης "
    L"(350-400 ευρώ ανά στρέμμα), στην εξαιρετική απόδοσή του που είναι 250-350 κιλά αποξηραμένα φύλλα και στο "
    L"εισόδημα που μπορεί να φτάσει τα 500-700 ευρώ το στρέμμα για συμβατικές καλλιέργειες και τα 1.500-1.800 "
    L"ευρώ για τις βιολογικές.",

    L"Μπορεί να αξιοποιήσει εγκαταλελειμμένες ορεινές και ημιορεινές εκτάσεις χωρίς ιδιαίτερες απαιτήσεις, "
    L"προσφέροντας ένα διόλου ευκαταφρόνητο συμπλήρωμα στο οικογενειακό εισόδημα. Πρόκειται για αυτόχθονο, "
    L"μακρόβιο δέντρο, γνωστό από την αρχαιότητα για τον καρπό του καθώς και για το ιδιαίτερα σκληρό του ξύλο."
    L" Με στρεμματική απόδοση 1.000 κιλά ανά στρέμμα, το κράνο μπορεί να αποφέρει περίπου 800-1.300 ευρώ καθαρά "
    L"έσοδα στον παραγωγό. Επίσης, το ξύλο της κρανιάς χρησιμοποιείται στην κατασκευή διαφόρων μικροκατασκευών "
    L"και εργαλείων (μπαστούνια, γκλίτσες, βέργες, πίπες κ.λπ.). Από τον φλοιό προέρχεται κόκκινη βαφή με την "
    L"οποία παλαιότερα βάφονταν δέρματα, ενώ με τους καρπούς έβαφαν αβγά. Επίσης παράγεται ένα παραδοσιακό λικέρ. "
    L"Το μόνο που ζητεί είναι νερό. Ειδικά όταν το δέντρο είναι νεαρό, χρειάζεται άφθονο νερό και το σωστό είναι "
    L"να αρδεύεται κάθε 10-15 ημέρες κατά τη θερινή, άνομβρη περίοδο.",

    L"Η στέβια είναι μια από τις λίγες συμβολαιακές καλλιέργειες που υπάρχουν αυτή τη στιγμή στην Ελλάδα, "
    L"αλλά και στην Ευρώπη γενικότερα. Η Ελλάδα θεωρείται «προνομιούχος» χώρα για την εν λόγω καλλιέργεια, "
    L"αφού η φωτοπερίοδος ευνοεί την ανάπτυξη του φυτού, δίνοντας εξαιρετικά μεγάλες παραγωγές σε σχέση με "
    L"άλλες χώρες, όπου το φυτό έχει το μισό ή και λιγότερο τελικό ύψος πριν την περίοδο της κοπής. Η ανάπτυξη "
    L"της στέβιας απαιτεί μικρές ποσότητες νερού και λιπασμάτων. Η τιμή των ξερών φύλλων είναι 1-2,5 ευρώ "
    L"και η απόδοση ανά στρέμμα 100-150 κιλά. Υπό ιδανικές συνθήκες, η παραγωγή ανά στρέμμα μπορεί να φτάσει "
    L"τα 400-450 κιλά και τα έσοδα να αγγίξουν τα 1.000 ευρώ το στρέμμα. Η προέλευση του φυτού υποδεικνύει "
    L"τις οικολογικές συνθήκες κάτω από τις οποίες μπορεί να ευδοκιμήσει και να αποτελέσει μια παραγωγική καλλιέργεια.",

    L"Μπορεί να μην εντάσσεται στις καθημερινές διατροφικές συνήθειες των Ελλήνων, ανήκοντας όμως στην κατηγορία "
    L"των super foods αρχίζει ήδη να γνωρίζει ζήτηση από τις βιομηχανίες μεταποίησης και σε συνδυασμό με τις "
    L"ικανοποιητικές αποδόσεις, σκαρφαλώνει ψηλά στη λίστα με τις πλέον προσοδοφόρες εναλλακτικές καλλιέργειες. "
    L"Το γκότζι μπέρι, ένα ξενόφερτο φρούτο που πριν από μερικά χρόνια το αγνοούσαμε στην Ελλάδα, αυξάνει "
    L"διαρκώς τις «μετοχές» του και αρχίζει να γίνεται αρεστό από καταναλωτές και βιομηχανίες, δημιουργώντας "
    L"ευνοϊκές συνθήκες ανάπτυξης για την καλλιέργεια του στη χώρα μας. Το φυλλοβόλο φυτό αντέχει τόσο σε υψηλές "
    L"θερμοκρασίες έως 40° C όσο και σε χαμηλές (όπως στους -30° C). Εντυπωσιακή είναι και η στρεμματική απόδοση "
    L"που φτάνει σε 1,5 τόνο νωπών καρπών, 700-800 αποξηραμένων. Η τιμή παραγωγού διαμορφώνεται σε 6-7 ευρώ το "
    L"κιλό αποξηραμένου καρπού, δίνοντας καθαρά έσοδα περίπου 2.000 ευρώ."
};

FARMER_DATA *farmers = NULL;

HINSTANCE g_hInst;

static TOOLINFOW toolInfo = { 0 };
static HWND hCropList = NULL, ttCropList;
static HBITMAP hBackground = NULL;
static BOOL charts_enabled = TRUE;
static BOOL fCropListTooltip = FALSE;
static BOOL fTrackingMouse = FALSE;
static POINT mouse_coords;

BOOL SetDoubleOnField(HWND hwnd, double val, int field_id);
BOOL GetDoubleFromField(HWND hwnd, double *val, int field_id);
BOOL SetFields(HWND hwnd, FIELD_DATA *fld);
BOOL GetFields(HWND hwnd, FIELD_DATA *fld);
BOOL NewFarmerEntry(HWND hwnd);
BOOL SetResults(HWND hwnd);

static INT_PTR OnDestroy(HWND hwnd);
static INT_PTR OnClose(HWND hwnd);
static INT_PTR OnCommand(HWND hwnd, int wParamLow, HWND hctl, UINT wParamHigh);
static INT_PTR OnInitDialog(HWND hwnd, HWND hkey, LPARAM lParam);


void EnableFields(HWND hwnd,
                  BOOL f)
{
    EnableWindow(GetDlgItem(hwnd, IDT_ACRES), f);
    EnableWindow(GetDlgItem(hwnd, IDT_FERTILIZER), f);
    EnableWindow(GetDlgItem(hwnd, IDT_SEED), f);
    EnableWindow(GetDlgItem(hwnd, IDT_ACRES_RENTED), f);
    EnableWindow(GetDlgItem(hwnd, IDT_PRICE_RENTED), f);
    EnableWindow(GetDlgItem(hwnd, IDT_MISC), f);
    EnableWindow(GetDlgItem(hwnd, IDT_TOOLS), f);
    EnableWindow(GetDlgItem(hwnd, IDC_WATERING), f);
    EnableWindow(GetDlgItem(hwnd, IDC_WATERING), f);
}

BOOL MouseIsOnCropList(HWND hwnd,
                       RECT *rcCropListOut)
{
    POINT pt;
    static RECT rcCropList;

    if (!GetWindowRect(hCropList, &rcCropList)) {
        MSGBOX_LASTERR(hwnd, L"GetWindowRect");
        DestroyWindow(hwnd);
        return TRUE;
    }

    if (rcCropListOut)
        *rcCropListOut = rcCropList;

    pt = (POINT) {.x = rcCropList.left, .y = rcCropList.top};
    ScreenToClient(hwnd, &pt);
    rcCropList.left = pt.x;
    rcCropList.top = pt.y;

    pt = (POINT) {.x = rcCropList.right, .y = rcCropList.bottom};
    ScreenToClient(hwnd, &pt);
    rcCropList.right = pt.x;
    rcCropList.bottom = pt.y;

    pt = mouse_coords;

    for (int i = 0; i < CROPLIST_MAXOFFS; i++) {
        if (pt.x > rcCropList.right) {
            pt.x -= 1;
        } else if (pt.x < rcCropList.left) {
            pt.x += 1;
        }

        if (pt.y > rcCropList.bottom) {
            pt.y -= 1;
        } else if (pt.y < rcCropList.top) {
            pt.y += 1;
        }

        if (pt.x <= rcCropList.right &&
            pt.x >= rcCropList.left &&
            pt.y <= rcCropList.bottom &&
            pt.y >= rcCropList.top) {
            return TRUE;
        }
    }

    return FALSE;
}

CROP_TYPE TranslateCroplistIndex(int idx)
{
    if (idx >= 2 && idx < 9) {
        return idx - 2;
    }

    if (idx >= 12) {
        return idx - 5;
    }

    return INV_CROP_TYPE;
}

int TranslateCroptype(CROP_TYPE ct)
{
    if (ct == INV_CROP_TYPE)
        return -1;

    if (ct <= LENTILS) {
        return ct + 2;
    }

    return ct + 5;
}

BOOL SetDoubleOnField(HWND hwnd,
                      double val,
                      int field_id)
{
    if (FAILED(StringCchPrintfA(char_buff, sizeof char_buff, "%.2f", val))) {
        MessageBoxExW(hwnd, L"Η StringCchPrintfA απέτυχε.",
                      L"Κάτι συνέβη!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
        return FALSE;
    }

    if (!MultiByteToWideChar(CP_UTF8, 0, char_buff, -1, wchar_buff, ARRAYSIZE(wchar_buff))) {
        MSGBOX_LASTERR(hwnd, L"MultiByteToWideChar");
        return FALSE;
    }

    if (!SetDlgItemTextW(hwnd, field_id, wchar_buff)) {
        MSGBOX_LASTERR(hwnd, L"SetDlgItemTextW");
        return FALSE;
    }

    return TRUE;
}

BOOL GetDoubleFromField(HWND hwnd,
                        double *val,
                        int field_id)
{
    if (!GetDlgItemTextW(hwnd, field_id, wchar_buff, ARRAYSIZE(wchar_buff))) {

        if (GetLastError()) {
            MSGBOX_LASTERR(hwnd, L"GetDlgItemTextW");
            return FALSE;
        } else {
            *val = 0;
        }

    } else {

        _set_errno(0);
        *val = wcstod(wchar_buff, NULL);
        if (errno) {
            MSGBOX_ERRNO(hwnd, L"wcstod");
            return FALSE;
        }

    }

    return TRUE;
}

BOOL SetFields(HWND hwnd,
               FIELD_DATA *fld)
{
    SendMessageW(hCropList, CB_SETCURSEL, (WPARAM)TranslateCroptype(fld->crop), 0);

    if (!SetDoubleOnField(hwnd, fld->acres, IDT_ACRES))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->fert_cost, IDT_FERTILIZER))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->seed_cost, IDT_SEED))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->rented_acres, IDT_ACRES_RENTED))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->rented_cost, IDT_PRICE_RENTED))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->misc_cost, IDT_MISC))
        return FALSE;

    if (!SetDoubleOnField(hwnd, fld->tools_cost, IDT_TOOLS))
        return FALSE;

    CheckDlgButton(hwnd, IDC_WATERING, fld->watering);

    return TRUE;
}

BOOL GetFields(HWND hwnd,
               FIELD_DATA *fld)
{
    int idx = SendMessageW(hCropList, CB_GETCURSEL, 0, 0);
    if (CB_ERR == idx)
        fld->crop = INV_CROP_TYPE;
    else
        fld->crop = TranslateCroplistIndex(idx);

    if (!GetDoubleFromField(hwnd, &fld->acres, IDT_ACRES))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->fert_cost, IDT_FERTILIZER))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->seed_cost, IDT_SEED))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->rented_acres, IDT_ACRES_RENTED))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->rented_cost, IDT_PRICE_RENTED))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->misc_cost, IDT_MISC))
        return FALSE;

    if (!GetDoubleFromField(hwnd, &fld->tools_cost, IDT_TOOLS))
        return FALSE;

    fld->watering = IsDlgButtonChecked(hwnd, IDC_WATERING);
    
    return TRUE;
}

BOOL NewFarmerEntry(HWND hwnd)
{
    if (!InitNameDialog(hwnd)) {
        if (GetLastError()) {
            MSGBOX_LASTERR(hwnd, L"InitNameDialog");
            return FALSE;
        } else {
            return TRUE;
        }
    }

    if (!curr_farmer)
        EnableFields(hwnd, TRUE);

    farmers[curr_farmer].fld.crop = INV_CROP_TYPE;

    SendDlgItemMessage(hwnd, IDL_FARMERS, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)farmers[curr_farmer].name);
    SendDlgItemMessage(hwnd, IDL_FARMERS, LB_SELECTSTRING, (WPARAM)curr_farmer, (LPARAM)farmers[curr_farmer].name);

    SetDlgItemTextW(hwnd, IDT_RESULTS, L"");

    return SetFields(hwnd, &farmers[curr_farmer].fld);
}

BOOL SetResults(HWND hwnd)
{
    if (FAILED(StringCchPrintf(wchar_buff, _CVTBUFSIZE,
                               L"Έσοδα: %.2f €\r\nΈξοδα: %.2f €\r\nΚέρδη: %.2f €",
                               farmers[curr_farmer].fld.income,
                               farmers[curr_farmer].fld.expenses,
                               farmers[curr_farmer].fld.profit))) {
        MessageBoxExW(hwnd, L"Η StringCchPrintf απέτυχε.",
                      L"Κάτι συνέβη!", MB_OK | MB_ICONERROR,
                      MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
        return FALSE;
    }

    if (!SetDlgItemTextW(hwnd, IDT_RESULTS, wchar_buff)) {
        MSGBOX_LASTERR(NULL, L"SendDlgItemMessageW");
        return FALSE;
    }

    return TRUE;
}

INT_PTR OnInitDialog(HWND hwnd,
                     HWND hkey,
                     LPARAM lParam)
{
    hCropList = GetDlgItem(hwnd, IDC_PLANT);
    if (!hCropList) {
        MSGBOX_LASTERR(hwnd, L"GetDlgItem");
        DestroyWindow(hwnd);
        return TRUE;
    }

    ttCropList = CreateWindowEx(0, TOOLTIPS_CLASS, NULL,
                                WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | TTS_NOFADE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                hwnd, NULL,
                                g_hInst, NULL);
    if (!ttCropList) {
        MSGBOX_LASTERR(hwnd, L"CreateWindowEx");
        DestroyWindow(hwnd);
        return TRUE;
    }

    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
    toolInfo.uId = (UINT_PTR)hCropList;
    toolInfo.lpszText = LPSTR_TEXTCALLBACK;

    SendMessageW(ttCropList, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    SendMessageW(ttCropList, TTM_SETMAXTIPWIDTH, 0, 300);
    SendMessageW(ttCropList, TTM_SETDELAYTIME, (WPARAM)TTDT_RESHOW, MAKELPARAM(10, 0));
    SendMessageW(ttCropList, TTM_SETDELAYTIME, (WPARAM)TTDT_INITIAL, MAKELPARAM(10, 0));

    hBackground = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BCKGRND), IMAGE_BITMAP, 0, 0,
                                     LR_DEFAULTSIZE);
    if (!hBackground) {
        MSGBOX_LASTERR(hwnd, L"LoadImage");
        DestroyWindow(hwnd);
        return TRUE;
    }

    SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_SETEVENTMASK, (WPARAM)0, (LPARAM)ENM_LINK);

    if (!RunWeatherThread(hwnd)) {
        DestroyWindow(hwnd);
        return TRUE;
    }

    if (!RunNewsfeedThread(hwnd)) {
        DestroyWindow(hwnd);
        return TRUE;
    }

    SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)L"Συμβατικές");
    SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)L"--------------------");

    for (size_t i = 0; i <= GOJIBERRY; i++) {
        int err = SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)crop_names[i]);

        if (i == LENTILS) {
            SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)L"");
            SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)L"Εναλλακτικές");
            SendDlgItemMessageW(hwnd, IDC_PLANT, CB_ADDSTRING, 0, (LPARAM)L"--------------------");
        }

        switch (err) {
        case CB_ERR:
        case CB_ERRSPACE:
            MSGBOX_LASTERR(hwnd, L"SendDlgItemMessageW");
            DestroyWindow(hwnd);
            return TRUE;
        }
    }

    for (int i = IDT_FERTILIZER; i <= IDT_RESULTS; i++) {
        SendDlgItemMessageW(hwnd, i, EM_SETLIMITTEXT, (WPARAM)MAX_LEN_DOUBLE_FIELD, 0);
    }

    SendDlgItemMessageW(hwnd, IDT_WEATHERLOCATION, EM_SETLIMITTEXT, (WPARAM)256, 0);
    EnableWindow(GetDlgItem(hwnd, IDB_CHART), charts_enabled);

    EnableFields(hwnd, FALSE);

    return TRUE;
}

INT_PTR OnCommand(HWND hwnd,
                  int wParamLow,
                  HWND hctl,
                  UINT wParamHigh)
{

    switch (wParamHigh) {
    case CBN_SELCHANGE:

        switch (wParamLow) {
        case IDL_FARMERS:
            if (total_farmers) {

                if (!GetFields(hwnd, &farmers[curr_farmer].fld)) {
                    DestroyWindow(hwnd);
                    return TRUE;
                }

                curr_farmer = SendMessage(hctl, LB_GETCURSEL, 0, 0);
                if (!SetFields(hwnd, &farmers[curr_farmer].fld)) {
                    DestroyWindow(hwnd);
                    return TRUE;
                }

                if (farmers[curr_farmer].fld.processed) {
                    if (!SetResults(hwnd))
                        DestroyWindow(hwnd);
                } else {
                    if (!SetDlgItemTextW(hwnd, IDT_RESULTS, L"")) {
                        MSGBOX_LASTERR(NULL, L"SendDlgItemMessageW");
                        DestroyWindow(hwnd);
                    }
                }
            }

            return TRUE;

        case IDC_PLANT:
            {
                int idx = SendMessage(hctl, CB_GETCURSEL, 0, 0);

                idx = (int)TranslateCroplistIndex(idx);
                farmers[curr_farmer].fld.crop = idx;

                if (idx <= GOJIBERRY && idx >= TOBACCO) {
                    SendMessage(hctl, CB_SELECTSTRING, (WPARAM)idx, (LPARAM)crop_names[idx]);
                } else {
                    SendMessage(hctl, CB_SETCURSEL, (WPARAM)-1, 0);
                }
            }

            return TRUE;
        default:
            break;
        }
    default:
        break;
    }

    switch (wParamLow) {
    case IDB_PROCESS:
        if (!total_farmers) {
            MessageBoxExW(hwnd, L"Δεν επιλέχτηκε αγρότης.", L"Σφάλμα!", MB_OK | MB_ICONERROR,
                          MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
        } else {

            if (SendDlgItemMessage(hwnd, IDL_FARMERS, LB_GETCURSEL, 0, 0) == LB_ERR) {
                MessageBoxExW(hwnd, L"Δεν επιλέχτηκε αγρότης.", L"Σφάλμα!", MB_OK | MB_ICONERROR,
                              MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            } else {
                if (!GetFields(hwnd, &farmers[curr_farmer].fld)) {
                    DestroyWindow(hwnd);
                    return TRUE;
                }

                if (farmers[curr_farmer].fld.crop == INV_CROP_TYPE) {
                    MessageBoxExW(hwnd, L"Δεν επιλέχτηκε είδος καλλιέργειας.", L"Σφάλμα!", MB_OK | MB_ICONERROR,
                                  MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
                } else {
                    ProcessFieldData(&farmers[curr_farmer].fld);

                    if (!SetResults(hwnd))
                        DestroyWindow(hwnd);
                }
            }

        }

        return TRUE;

    case IDB_NEWFARMER:
        if (total_farmers >= MAX_FARMERS) {
            MessageBoxExW(hwnd, L"Ο αριθμός των αγροτών δεν επιτρέπεται "
                          L"να υπερβεί τους " MAX_FARMERS_WSTR L" αγρότες.",
                          L"Σφάλμα!", MB_OK | MB_ICONERROR,
                          MAKELANGID(LANG_GREEK, SUBLANG_GREEK_GREECE));
            return TRUE;
        }

        if (curr_farmer >= 0 && !GetFields(hwnd, &farmers[curr_farmer].fld)) {
            DestroyWindow(hwnd);
            return TRUE;
        }

        if (!NewFarmerEntry(hwnd))
            DestroyWindow(hwnd);

        return TRUE;

    case IDOK:
    case IDB_CHART:
        if (GetFocus() == GetDlgItem(hwnd, IDT_WEATHERLOCATION)) {
            GetWeather();
            
        } else if (charts_enabled && curr_farmer >= 0) {
            if (!GetFields(hwnd, &farmers[curr_farmer].fld)) {
                DestroyWindow(hwnd);
                return TRUE;
            }

            ProcessFieldData(&farmers[curr_farmer].fld);
            if (farmers[curr_farmer].fld.crop != INV_CROP_TYPE) {
                if (!SetResults(hwnd))
                    DestroyWindow(hwnd);
            }

            if (!InitChartDialog(hwnd)) {
                DestroyWindow(hwnd);
            }
        }

        return TRUE;

    }

    return FALSE;
}

INT_PTR OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    BITMAP bitmap;
    HDC hdcMem;
    HGDIOBJ prevBmp;

    hdc = BeginPaint(hwnd, &ps);

    hdcMem = CreateCompatibleDC(hdc);
    prevBmp = SelectObject(hdcMem, hBackground);

    GetObject(hBackground, sizeof(bitmap), &bitmap);
    BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, prevBmp);
    DeleteDC(hdcMem);

    EndPaint(hwnd, &ps);

    return TRUE;
}

INT_PTR OnClose(HWND hwnd)
{
    DestroyWindow(hwnd);
    return TRUE;
}

INT_PTR OnDestroy(HWND hwnd)
{
    PostQuitMessage(EXIT_SUCCESS);
    return TRUE;
}

void OpenLink(HWND hwnd,
              CHARRANGE cr)
{
    wchar_t *__tmp_buff;
    ULONG len;

    if ((cr.cpMin != cr.cpMax) && (cr.cpMax != -1)) {

        len = cr.cpMax - cr.cpMin;
        if (len > (ARRAYSIZE(wchar_buff) - 1)) {
            __tmp_buff = win_malloc((len + 1) * sizeof(wchar_t));
            if (!__tmp_buff) {
                MSGBOX_LASTERR(hwnd, L"win_malloc");
                return;
            }
        } else {
            __tmp_buff = wchar_buff;
        }

        WaitForSingleObject((HANDLE)newsfeed_lock, INFINITE);

        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_EXSETSEL , (WPARAM)0, (LPARAM)&cr);
        SendDlgItemMessageW(hwnd, IDT_NEWSFEED, EM_GETSELTEXT , (WPARAM)0, (LPARAM)__tmp_buff);

        ReleaseMutex(newsfeed_lock);

        ShellExecuteW(NULL, L"open", __tmp_buff, NULL, NULL, SW_SHOW);

        if (__tmp_buff != wchar_buff) {
            win_free(__tmp_buff);
        }
    }
}

void StartMouseTracking(HWND hwnd)
{
    TRACKMOUSEEVENT tme = { .cbSize = sizeof(TRACKMOUSEEVENT) };
    tme.hwndTrack       = hwnd;
    tme.dwFlags         = TME_LEAVE;

    _TrackMouseEvent(&tme);
}

INT_PTR OnMouseLeave(HWND hwnd)
{
    static RECT tmp;
    fTrackingMouse = FALSE;

    if (MouseIsOnCropList(hwnd, &tmp)) {
        fCropListTooltip = TRUE;
        SendMessage(ttCropList, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&toolInfo);
        SendMessage(ttCropList, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(tmp.right, tmp.top));
    }

    return FALSE;
}

INT_PTR OnMouseMove(HWND hwnd,
                    int cx,
                    int cy,
                    UINT id)
{
    mouse_coords = (POINT){.x = cx, .y = cy};

    if (!fTrackingMouse) {
        StartMouseTracking(hwnd);
        fTrackingMouse = TRUE;
    }

    if (fCropListTooltip) {
        SendMessage(ttCropList, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&toolInfo);
        fCropListTooltip = FALSE;
    }

    return FALSE;
}

INT_PTR OnNotify(HWND hwnd,
                 NMHDR *nmhdr)
{
    switch (nmhdr->code) {
    case TTN_NEEDTEXT:
        {
            LPNMTTDISPINFOW pInfo = (LPNMTTDISPINFOW)nmhdr;

            if (curr_farmer >= 0 && farmers[curr_farmer].fld.crop >= 0) {
                pInfo->lpszText = crop_description[farmers[curr_farmer].fld.crop];
            } else {
                pInfo->lpszText = LPSTR_TEXTCALLBACK;
            }
        }
        break;
    case EN_LINK:
        {
            ENLINK *penLink = (ENLINK *)nmhdr;

            if (penLink->msg == WM_LBUTTONUP) {
                OpenLink(hwnd, penLink->chrg);
            }
        }
        break;
    default:
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK MainDialogProcedure(HWND hwnd,
                                     UINT msg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
    switch (msg) {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
        HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMouseMove);
        case WM_NOTIFY: return OnNotify(hwnd, (NMHDR *)lParam);
        case WM_MOUSELEAVE: return OnMouseLeave(hwnd);
        HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    }

    return FALSE;
}

void Cleanup(void)
{
    for (ULONG i = 0; i < total_farmers; i++) {
        if (farmers[i].name)
            win_free(farmers[i].name);
    }

    win_free(farmers);

    if (charts_enabled)
        mcChart_Terminate();

    curl_global_cleanup();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   INT nCmdShow)
{
    MSG msg;
    BOOL bRet;
    HWND main_dlg;

    g_hInst = hInstance;

    curl_global_init(CURL_GLOBAL_ALL);

    INITCOMMONCONTROLSEX controls = {sizeof(INITCOMMONCONTROLSEX),
                                     ICC_BAR_CLASSES | ICC_LINK_CLASS | ICC_STANDARD_CLASSES | ICC_INTERNET_CLASSES};
    InitCommonControlsEx(&controls);

    _onexit((_onexit_t)Cleanup);
    _onexit((_onexit_t)StopWeatherThread);
    _onexit((_onexit_t)StopNewsfeedThread);

    if(!mcChart_Initialize()) {
        MessageBoxW(NULL, L"Η αρχικοποίηση των εργαλείου ελέγχου ιστογραμμάτων απέτυχε. "
                    L"Μάλλον δεν βρέθηκε το GDIPLUS.DLL στο μηχάνημα."
                    L"Η δημιουγία ιστογραμμάτων δεν θα είναι δυνατή.",
                    L"Προειδοποίηση", MB_OK | MB_ICONEXCLAMATION);
        charts_enabled = FALSE;
    }

    if (charts_enabled) {
        _onexit((_onexit_t)StopChartThread);

        if (!RunChartThread())
            exit(EXIT_FAILURE);
    }

    //Initialize the farmer list
    farmers = win_malloc(sizeof(FARMER_DATA) * farmer_array_len);
    if (!farmers) {
        MSGBOX_LASTERR(NULL, L"win_malloc");
        exit(EXIT_FAILURE);
    }

    if (!LoadLibraryW(L"Msftedit.dll")) {
        MSGBOX_LASTERR(NULL, L"LoadLibraryA");
        exit(EXIT_FAILURE);
    }

    main_dlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDialogProcedure);
    if (!main_dlg) {
        MSGBOX_LASTERR(NULL, L"CreateDialog");
        exit(EXIT_FAILURE);
    }

    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != FALSE) {
        if (bRet == -1) {
            MSGBOX_LASTERR(NULL, L"GetMessage");
            exit(EXIT_FAILURE);
        }

        if (!IsDialogMessage(main_dlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    exit(msg.wParam);
}
