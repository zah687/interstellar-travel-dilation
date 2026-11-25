/*  Space Travel Calculator
 
  how long a trip takes:
    Earth time: how many years pass for people staying on Earth
    Ship time:  how many years pass for the crew (time slows down on a fast ship)
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Destination {
    const char *name;
    double light_years;
};

static const struct Destination destinations[] = {
    {"Moon",              0.0000000406},   // 384,400 km 
    {"Venus",             0.00000402},     /* closest approach, around 38 million km */
    {"Mars",              0.00000577},     /* closest approach, around 55 million km */
    {"Jupiter",           0.0000622},
    {"Saturn",            0.000127},
    {"Neptune",           0.000455},
    {"Proxima Centauri",  4.24},
    {"Sirius",            8.6},
    {"Tau Ceti",          11.9},
    {"Vega",              25},
    {"TRAPPIST-1",        40},
    {"Pleiades",          444},
    {"Galactic centre",   26000},
};

/* How many entries are in the list: total bytes / bytes of 1 entry */
#define COUNT (int)(sizeof(destinations) / sizeof(destinations[0]))

/* Convert text to a number. Returns 1 if okay , 0 if the text is not a number */
static int parse(const char *text, double *out) {
    char *end;
    *out = strtod(text, &end);
    return end != text && *end == '\0';
}

static void print_time(const char *label, double years) {
    double days = years * 365.25;
    if (years >= 1)           printf("%s: %g years", label, years);
    else if (days >= 1)       printf("%s: %g days", label, days);
    else if (days * 24 >= 1)  printf("%s: %g hours", label, days * 24);
    else if (days * 24 * 60 >= 1)            printf("%s: %g minutes", label, days * 24 * 60);
    else                       printf("%s: %g seconds", label, days * 24 * 60 * 60);
}

// Performs validation on the user input

static int run_trip(double choice, double percent) {
    if (choice != floor(choice) || choice < 1 || choice > COUNT) {
        fprintf(stderr, "Invalid! destination must be a whole number from 1 to %d.\n", COUNT);
        return 2;
    }
    // cannot be 100 percent since it is physically impossible to travel at the speed of light
    if (percent <= 0 || percent >= 100) {
        fprintf(stderr, "Speed must be above 0 and below 100 percent.\n");
        return 2;
    }

    const struct Destination *d = &destinations[(int)choice - 1];
    double v = percent / 100.0;
   // How long the journey takes for someone staying on Earth
    double earth_years = d->light_years / v;

    // Time dilation: less time passes on the ship using sqrt(1 - v^2)
    double ship_years = earth_years * sqrt(1.0 - v * v);

    printf("%s (%g ly) at %g%% of light speed: ", d->name, d->light_years, percent);
    print_time("Earth time", earth_years);
    printf(" | ");
    print_time("Ship time", ship_years);
    printf("\n");
    return 0;
}



int main(int argc, char *argv[]) {
    double choice, percent;

    /* With arguments (used by Docker and the website): one calculation then stop */
    if (argc == 3) {
        if (!parse(argv[1], &choice) || !parse(argv[2], &percent)) {
            fprintf(stderr, "Invalid! destination and speed must be numbers.\n");
            return 1;
        }
        return run_trip(choice, percent);
    }

    /* keep asking until the user types 0 */
    while (1) {
        printf("\nWhere do you want to go?\n");
        for (int i = 0; i < COUNT; i++) {
            printf("  %d) %s (%g light years)\n", i + 1, destinations[i].name,
                   destinations[i].light_years);
        }
        printf("  0) Quit\n");
        printf("Please enter the destination number: ");
        fflush(stdout);

        if (scanf("%lf", &choice) != 1) {
            fprintf(stderr, "Invalid number.\n");
            return 1;
        }
        if (choice == 0) {
            printf("End of Program, goodbye. \n");
            return 0;
        }

        printf("Enter the speed in %% of light speed: ");
        fflush(stdout);
        if (scanf("%lf", &percent) != 1) {
            fprintf(stderr, "Invalid number.\n");
            return 1;
        }

        run_trip(choice, percent);   /* prints the result or an error then we loop again */
    }
}