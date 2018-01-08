#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <gmp.h>
#include "algorithms/multiset_combination.h"
#include "utils.h"

double _ncomb_f(int* f, size_t flen, size_t k);

SEXP next_multiset_combinations(SEXP _n, SEXP _k, SEXP _d, SEXP state, SEXP labels, SEXP f, SEXP _type) {
    size_t i, j, h;

    size_t n = as_uint(_n);
    size_t k = as_uint(_k);
    int d;
    double dd;
    if (Rf_asInteger(_d) == -1) {
        dd = _ncomb_f(INTEGER(f), Rf_length(f), k);
    } else {
        dd = as_uint(_d);
    }

    int ltype = TYPEOF(labels);
    int* labels_intp;
    double* labels_doublep;

    int* fp;

    char type = CHAR(Rf_asChar(_type))[0];

    if (type == 'l') {
        if (dd > INT_MAX) Rf_error("too many results");
    } else {
        if (dd * k > INT_MAX) Rf_error("too many results");
    }
    d = round(dd);

    SEXP ms;
    unsigned int* mp;
    SEXP as;
    unsigned int* ap;

    int status = 0;

    if (state == R_NilValue) {
        ms = R_UnboundValue;
        as = R_UnboundValue;
    } else {
        ms = Rf_findVarInFrame(state, Rf_install("m"));
        as = Rf_findVarInFrame(state, Rf_install("a"));
    }

    if (ms == R_UnboundValue) {
        if (state == R_NilValue) {
            mp = (unsigned int*) R_alloc(n, sizeof(int));
        } else {
            ms = PROTECT(Rf_allocVector(INTSXP, n));
            Rf_defineVar(Rf_install("m"), ms, state);
            UNPROTECT(1);
            mp = (unsigned int*) INTEGER(ms);
        }
        fp = INTEGER(f);
        h = 0;
        for (i = 0; i< Rf_length(f); i++) {
            for (j = 0; j< fp[i]; j++) {
                mp[h++] = i;
            }
        }

    } else {
        mp = (unsigned int*) INTEGER(ms);
    }

    if (as == R_UnboundValue) {
        if (state == R_NilValue) {
            ap = (unsigned int*) R_alloc(k, sizeof(int));
        } else {
            as = PROTECT(Rf_allocVector(INTSXP, k));
            Rf_defineVar(Rf_install("a"), as, state);
            UNPROTECT(1);
            ap = (unsigned int*) INTEGER(as);
        }
        fp = INTEGER(f);
        h = 0;
        for (i = 0; i< k; i++) {
            ap[i] = mp[i];
        }

    } else {
        ap = (unsigned int*) INTEGER(as);
        status = 1;
    }

    SEXP rdim;
    SEXP result, resulti;
    int* result_intp;
    double* result_doublep;
    int nprotect = 0;

    if (type == 'r') {
        if (labels == R_NilValue) {
            result = PROTECT(Rf_allocVector(INTSXP, k*d));
            nprotect++;
            result_intp = INTEGER(result);
        } else {
            result = PROTECT(Rf_allocVector(ltype, k*d));
            nprotect++;
            if (ltype == INTSXP) {
                result_intp = INTEGER(result);
                labels_intp = INTEGER(labels);
            } else if (ltype == REALSXP) {
                result_doublep = REAL(result);
                labels_doublep = REAL(labels);
            }
        }

        for (j=0; j<d; j++) {
            if (status) {
                if (!next_multiset_combination(mp, ap, n, k)) {
                    status = 0;
                    break;
                }
            } else {
                status = 1;
            }
            if (ltype == NILSXP) {
                for (i=0; i<k; i++) {
                    result_intp[j + i*d] = ap[i] + 1;
                }
            } else if (ltype == INTSXP) {
                for (i=0; i<k; i++) {
                    result_intp[j + i*d] = labels_intp[ap[i]];
                }
            } else if (ltype == REALSXP) {
                for (i=0; i<k; i++) {
                    result_doublep[j + i*d] = labels_doublep[ap[i]];
                }
            } else if (ltype == STRSXP) {
                for (i=0; i<k; i++) {
                    SET_STRING_ELT(result, j + i*d, STRING_ELT(labels, ap[i]));
                }
            }
        }
        if (status == 0) {
            result = PROTECT(resize_row(result, k, d, j));
            nprotect++;
        }
        PROTECT(rdim = Rf_allocVector(INTSXP, 2));
        INTEGER(rdim)[0] = j;
        INTEGER(rdim)[1] = k;
        Rf_setAttrib(result, R_DimSymbol, rdim);
        UNPROTECT(1);

    } else if (type == 'c') {
        if (labels == R_NilValue) {
            result = PROTECT(Rf_allocVector(INTSXP, k*d));
            nprotect++;
            result_intp = INTEGER(result);
        } else {
            result = PROTECT(Rf_allocVector(ltype, k*d));
            nprotect++;
            if (ltype == INTSXP) {
                result_intp = INTEGER(result);
                labels_intp = INTEGER(labels);
            } else if (ltype == REALSXP) {
                result_doublep = REAL(result);
                labels_doublep = REAL(labels);
            }
        }

        for (j=0; j<d; j++) {
            if (status) {
                if (!next_multiset_combination(mp, ap, n, k)) {
                    status = 0;
                    break;
                }
            } else {
                status = 1;
            }
            if (ltype == NILSXP) {
                for (i=0; i<k; i++) {
                    result_intp[j * k + i] = ap[i] + 1;
                }
            } else if (ltype == INTSXP) {
                for (i=0; i<k; i++) {
                    result_intp[j * k + i] = labels_intp[ap[i]];
                }
            } else if (ltype == REALSXP) {
                for (i=0; i<k; i++) {
                    result_doublep[j * k + i] = labels_doublep[ap[i]];
                }
            } else if (ltype == STRSXP) {
                for (i=0; i<k; i++) {
                    SET_STRING_ELT(result, j * k + i, STRING_ELT(labels, ap[i]));
                }
            }
        }
        if (status == 0) {
            result = PROTECT(resize_col(result, k, d, j));
            nprotect++;
        }
        PROTECT(rdim = Rf_allocVector(INTSXP, 2));
        INTEGER(rdim)[0] = k;
        INTEGER(rdim)[1] = j;
        Rf_setAttrib(result, R_DimSymbol, rdim);
        UNPROTECT(1);

    } else {
        // type == "list"
        result = PROTECT(Rf_allocVector(VECSXP, d));
        nprotect++;
        if (ltype == INTSXP) {
            labels_intp = INTEGER(labels);
        } else if (ltype == REALSXP) {
            labels_doublep = REAL(labels);
        }

        for (j=0; j<d; j++) {
            if (status) {
                if (!next_multiset_combination(mp, ap, n, k)) {
                    status = 0;
                    break;
                }
            } else {
                status = 1;
            }
            if (ltype == NILSXP) {
                resulti = Rf_allocVector(INTSXP, k);
                result_intp = INTEGER(resulti);
                for (i=0; i<k; i++) {
                    result_intp[i] = ap[i] + 1;
                }
            } else if (ltype == INTSXP) {
                resulti = Rf_allocVector(INTSXP, k);
                result_intp = INTEGER(resulti);
                for (i=0; i<k; i++) {
                    result_intp[i] = labels_intp[ap[i]];
                }
            } else if (ltype == REALSXP) {
                resulti = Rf_allocVector(REALSXP, k);
                result_doublep = REAL(resulti);
                for (i=0; i<k; i++) {
                    result_doublep[i] = labels_doublep[ap[i]];
                }
            } else if (ltype == STRSXP) {
                resulti = Rf_allocVector(STRSXP, k);
                for (i=0; i<k; i++) {
                    SET_STRING_ELT(resulti, i, STRING_ELT(labels, ap[i]));
                }
            }
            SET_VECTOR_ELT(result, j, resulti);
        }
        if (status == 0) {
            result = PROTECT(resize_list(result, d, j));
            nprotect++;
        }
    }

    UNPROTECT(nprotect);
    return result;
}


double _ncomb_f(int* f, size_t flen, size_t k) {
    int n = 0;
    int i, j, h;
    for (i=0; i<flen; i++) n += f[i];
    if (k > n) {
        return 0;
    }

    double* p = (double*) malloc((k+1) * sizeof(double));
    for (j=0; j<=k; j++) p[j] = 0;

    double ptemp;

    for (i=0; i<flen; i++) {
        if (i == 0) {
            for (j=0; j<=k && j<=f[i]; j++) {
                p[j] = 1;
            }
            ptemp = p[k];
        } else if (i < flen - 1){
            for (j=k; j>0; j--) {
                ptemp = 0;
                for(h=0; h<=f[i] && h<=j; h++) {
                    ptemp += p[j-h];
                }
                p[j] = ptemp;
            }
        } else {
            ptemp = 0;
            for(h=0; h<=f[i] && h<=k; h++) {
                ptemp += p[k-h];
            }
        }
    }

    free(p);
    return ptemp;
}

SEXP ncomb_f(SEXP f, SEXP _k) {
    int* fp = INTEGER(f);
    size_t flen = Rf_length(f);
    size_t k = as_uint(_k);
    return Rf_ScalarReal(_ncomb_f(fp, flen, k));
}

char* _ncomb_f_bigz(int* f, size_t flen, size_t k) {
    int n = 0;
    int i, j, h;
    char* out;
    for (i=0; i<flen; i++) n += f[i];
    if (k > n) {
        out = (char*) malloc(sizeof(char));
        out[0] = '0';
        return out;
    }

    mpz_t* p = (mpz_t*) malloc((k+1) * sizeof(mpz_t));
    for (j=0; j<=k; j++) mpz_init(p[j]);

    mpz_t ptemp;
    mpz_init(ptemp);

    for (i=0; i<flen; i++) {
        if (i == 0) {
            for (j=0; j<=k && j<=f[i]; j++) {
                mpz_set_ui(p[j], 1);
            }
            mpz_set(ptemp, p[k]);
        } else if (i < flen - 1){
            for (j=k; j>0; j--) {
                mpz_set_ui(ptemp, 0);
                for(h=0; h<=f[i] && h<=j; h++) {
                    mpz_add(ptemp, ptemp, p[j-h]);
                }
                mpz_set(p[j], ptemp);
            }
        } else {
            mpz_set_ui(ptemp, 0);
            for(h=0; h<=f[i] && h<=k; h++) {
                mpz_add(ptemp, ptemp, p[k-h]);
            }
        }
    }

    out = mpz_get_str(NULL, 10, ptemp);
    for (j=0; j<=k; j++) mpz_clear(p[j]);
    free(p);
    mpz_clear(ptemp);
    return out;
}

SEXP ncomb_f_bigz(SEXP f, SEXP _k) {
    int* fp = INTEGER(f);
    size_t flen = Rf_length(f);
    size_t k = as_uint(_k);
    char* c = _ncomb_f_bigz(fp, flen, k);
    SEXP out = Rf_mkString(c);
    free(c);
    return out;
}
