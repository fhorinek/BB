
////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void)(x)

static inline uint32_t op_n(int32_t val) { return (uint32_t)(val < 0 ? -val : val); }
static inline uint32_t op_i(uint32_t val) { return val; }
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val) { UNUSED(val); return 0;}
static inline uint32_t op_w(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_f(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_t(uint32_t val) { UNUSED(val); return 0; }

static lv_i18n_phrase_t de_de_singulars[] = {
    {"Flightbook", "Flugbuch"},
    {"Airspaces", "Lufträume"},
    {"Language", "Sprache"},
    {"Display Settings", "Display-Einstellungen"},
    {"Brightness", "Helligkeit"},
    {"Page animation", "Seitenanimation"},
    {"Battery percent", "Batterieprozent"},
    {NULL, NULL} // End mark
};



static uint8_t de_de_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t de_de_lang = {
    .locale_name = "de-DE",
    .singulars = de_de_singulars,

    .locale_plural_fn = de_de_plural_fn
};

static lv_i18n_phrase_t sk_sk_singulars[] = {
    {"Flightbook", "Letové záznamy"},
    {"Airspaces", "Vzdušné priestory"},
    {"Language", "Jazyk"},
    {"Display Settings", "Nastavenie displeja"},
    {"Brightness", "Jas"},
    {"Page animation", "Animácie stránok"},
    {"Battery percent", "Percentá batérie"},
    {"Strato settings", "Nastavenia"},
    {"Pilot & Flight profile", "Profily"},
    {"Vario", "Vário"},
    {"Flight", "Let"},
    {"Map", "Mapa"},
    {"System", "Systém"},
    {"Development", "Vývoj"},
    {NULL, NULL} // End mark
};



static uint8_t sk_sk_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    if (((2 <= i && i <= 4) && v == 0)) return LV_I18N_PLURAL_TYPE_FEW;
    if ((v != 0)) return LV_I18N_PLURAL_TYPE_MANY;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t sk_sk_lang = {
    .locale_name = "sk-SK",
    .singulars = sk_sk_singulars,

    .locale_plural_fn = sk_sk_plural_fn
};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
    &de_de_lang,
    &sk_sk_lang,
    NULL // End mark
};
