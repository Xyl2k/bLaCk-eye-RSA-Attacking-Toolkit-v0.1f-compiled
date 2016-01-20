/*
    RSA Attacking Toolkitt v.0.1f

    - just a little program to compute the factors of the moduls given the private and public key  
    - it's very crude, if you want more, you have the source,Do it!
    - future plans: to have threads working.

    START PROJECT   :  11 March 10:50
    Author          :  blaCk-eye / mycherynos@yahoo.com
    
*/
#include <stdio.h>
#include <windows.h>
#include <commctrl.h> 
#include "resource.h"
#include <big.h>   /* include MIRACL system */
#include "big.cpp"
#include "md5.c"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "miracl.lib")

Miracl      precision(5000,16);     /* bigs are 5000 decimal digits long */
HINSTANCE	hInst;
long        attack_type=IDC_CMA;    /* the type of attack*/    
int         no_base=0;
miracl *mip;

BOOL CMA_Attack(HWND hWnd)
{
    /* Commun Modulus Attack */

    Big n=0,e=0,d=0,p=0,q=0,k=0,a=0,r=1,g=0;
    unsigned long s=0;
    long t;
    unsigned int i,j=1;;
    bool succes=false;
    char sztemp[2048];
    int idc[]={IDC_N,IDC_E,IDC_D};

    /* check if require keys have been entered */    
    for (i=0, j=1; i<3; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        /* get required keys */
        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_D, (char*) sztemp, 2048);
        d = sztemp;
        GetDlgItemText(hWnd, IDC_E, (char*) sztemp, 2048);
        e = sztemp;

        /* start of algo*/
        //initialise random number seed
        irand(GetTickCount());
        
        k = e*d-1;
        s=0;
        while (k%2==0)
        {
            k = k/2;
            s += 1;
        }
        do
        {
            
            a = rand(n);
            if (gcd(a,n)!=1)
            {
                p = a;
                q = n/a;
                if ( p * q == n)
                {
                    succes = true;
                }
            }
            else
            {
                for (t=s-1; t>=0; t--)
                {
                    
                    r = pow((Big)2,t) * k;
                    r = (pow(a,r,n) - 1)%n;
                    
                    if (n > gcd(r,n) && gcd(r,n) > 1)
                    {
                        p = gcd(r,n);
                        q = n/gcd(r,n);
                        if ( p*q==n )
                        {
                            succes = true;
                            break;
                        }
                    }
                }
            }
            if (succes)
                break;
        }
        while (1);
        /* end of algo*/

        /* print values */
        sztemp << p;
        SetDlgItemText(hWnd, IDC_P, ((char*) sztemp));
        sztemp << q;
        SetDlgItemText(hWnd, IDC_Q, ((char*) sztemp));
        return true;

    }
    else
    {
        MessageBox(hWnd,"Please fill all the fields to try the attack!", "Upss..", MB_ICONERROR);
        return false;    
    }
}

BOOL Wener_Attack(HWND hWnd)
{
    /* Small prime difference attack a.k.a Wener attack */

    Big n,e,x,y,p,q,d,u,v;
    int idc[]={IDC_N,IDC_E};
    unsigned int i,j;
    char sztemp[2048];

    /* check if require keys have been entered */    
    for (i=0, j=1; i<2; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        /* get required keys */
        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_E, (char*) sztemp, 2048);
        e = sztemp;
        
        /* start of algo*/
        for (x=2*sqrt(n);;x+=1)
        {
            u = pow(x,2) - 4 * n;
            if (u > 1)
            {
                v = sqrt(u);
                if ( pow(v,2) == u )
                {
                    y = sqrt(pow(x,2) - 4*n);

                    p = (x+y)/2;
                    q = (x-y)/2;
                    break;
                }
            }
        }     
        
        /* end of algo*/

        /* print values */
        sztemp << p;
        SetDlgItemText(hWnd, IDC_P, ((char*) sztemp));
        sztemp << q;
        SetDlgItemText(hWnd, IDC_Q, ((char*) sztemp));
        sztemp << inverse(e,(p-1)*(q-1));;
        SetDlgItemText(hWnd, IDC_D, ((char*) sztemp));
        return true;    

    }
    else
    {     
        MessageBox(hWnd,"Please fill all the fields to try the attack!", "Upss..", MB_ICONERROR);
        return false;    
    }

}

BOOL Wiener_Attack(HWND hWnd)
{
    /* 
        Wiener Attack on RSA (small private exponent)
        works if  q < p < 2*q and d < 1/3*(n)^(1/4)
    */
    
    Big n,e,d,p,q,pj,qj,pk,qk,ai,bi,aj,bj,pi,qi,l,crt,sum,dif;
    int idc[]={IDC_N,IDC_E};
    unsigned int i,j;
    char sztemp[2048];
    bool succes=FALSE;

    /* check if require keys have been entered */    
    for (i=0, j=1; i<2; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        /* get required keys */
        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_E, (char*) sztemp, 2048);
        e = sztemp;
        
        /* start of algo : continue fraction method*/

        /* calculate l = 1/3*(n)^(1/4) */
        l = root(n,4)/3;
        /* initiliase: */
        crt = n;
        aj = 0;
        bj = e;
        ai = crt/bj;
        bi = crt%bj;
        crt= bj;

        pk = 0;
        qk = 1;
        pj = ai * aj + 1;
        qj = ai;
        
        for (;;)
        {
            aj = crt/bi;
            bj = crt%bi;
            crt = bi;

            pi = aj * pj + pk;
            qi = aj * qj + qk;
            
            if (qi > l)
            {
                succes = false;
                break;
            }
            sum = n - (qi*e-1)/pi + 1;
            dif = sqrt((sum/2)*(sum/2) - n)*2;
            p = (sum+dif)/2;
            q = (sum-dif)/2;
            if (p*q == n)
            {
                    succes = TRUE;
                    break;
            }
            ai = bi;
            bi = bj;
            pk = pj;
            pj = pi;
            qk = qj;
            qj = qi;
        }
        
        /* end of algo*/
        if (succes)
        {
            /* print values */
            sztemp << p;
            SetDlgItemText(hWnd, IDC_P, ((char*) sztemp));
            sztemp << q;
            SetDlgItemText(hWnd, IDC_Q, ((char*) sztemp));
            sztemp << inverse(e,(p-1)*(q-1));
            SetDlgItemText(hWnd, IDC_D, ((char*) sztemp));
        }
        else
        {
            MessageBox(hWnd,"This RSA key isn't susceptible to Wiener attack!\nPossible reason: D > (1/3)*N^(1/4)!","Failure...",MB_ICONINFORMATION);
        }
        return succes;    

    }
    else
    {     
        MessageBox(hWnd,"Please fill all the fields to try the attack!", "Upss..", MB_ICONERROR);
        return false;    
    }

    return false;
}

void Low_Attack(HWND hWnd)
{
    /* Low Message Attack */
    /*
        If the encryption exponent is small e=3
        and m is small enough so that m^e < N then we
        can directly calculate m by computing the e'th root
        from the cipher text
    */

    int idc[]={IDC_N,IDC_CIPHER};
    Big n,e,c,t;
    int i,j;
    char sztemp[2048];

    /* check if require keys have been entered */    
    for (i=0, j=1; i<2; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        /* get required keys */

        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_CIPHER, (char*) sztemp, 2048);
        c = sztemp;

        /* start of algo*/

        /* compute e'th root of ciphertext*/
        t = root(c,3);

        /* see if it's correct */
        if ( pow(t,3) == c )
        {
            RtlZeroMemory((char*) sztemp, 2048);
            to_binary(t, 2048, (char*) sztemp, FALSE);
            SetDlgItemText(hWnd, IDC_PLAIN, (char*) sztemp);
        }
        else
            MessageBox(hWnd,"The key isn't susceptible to this attack. Probably M^E > N", "Upss...", MB_ICONERROR);
        /* end of algo*/
    }
    else
        MessageBox(hWnd,"Please fill all the necessary fields", "Upss..", MB_ICONERROR);
}

void DisableKey(HWND hWnd)
{
    /* disables all RSA private keys editboxes (D,P,Q)*/

    int idc[]={IDC_D,IDC_E,IDC_P,IDC_Q};
    for (int i=0; i<3; i++)
    {
        SendMessage(GetDlgItem(hWnd,idc[i]), EM_SETREADONLY, TRUE, NULL);
    }
}

void RSA_Encrypt(HWND hWnd)
{
    Big m,n,e;
    char sztemp[2048];
    int idc[]={IDC_N, IDC_E, IDC_PLAIN};
    unsigned int i,j;

    /* check if require keys have been entered */    
    for (i=0, j=1; i<3; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_E, (char*) sztemp, 2048);
        e = sztemp;
        GetDlgItemText(hWnd, IDC_PLAIN, (char*) sztemp, 2048);
        m = from_binary(strlen((char*) sztemp), (char*) sztemp);
        m = pow(m,e,n);
        sztemp << m;
        SetDlgItemText(hWnd, IDC_CIPHER, (char*) sztemp);
    }
    else
        MessageBox(hWnd,"Please fill all the necessary fields", "Upss..", MB_ICONERROR);
}

void RSA_Decrypt(HWND hWnd)
{
    Big m,n,d;
    char sztemp[2048];
    int idc[]={IDC_N,IDC_D, IDC_CIPHER};
    unsigned int i,j;

    /* check if require keys have been entered */    
    for (i=0, j=1; i<3; i++)
    {
        if ( GetWindowTextLength(GetDlgItem(hWnd, idc[i])) == 0 )
            j = 0;
    }
    if (j)
    {
        GetDlgItemText(hWnd, IDC_N, (char*) sztemp, 2048);
        n = sztemp;
        GetDlgItemText(hWnd, IDC_D, (char*) sztemp, 2048);
        d = sztemp;
        GetDlgItemText(hWnd, IDC_CIPHER, (char*) sztemp, 2048);
        m = sztemp;
        m = pow(m,d,n);
        RtlZeroMemory((char*) sztemp, 2048);
        to_binary(m, 2048, (char*) sztemp, FALSE);
        SetDlgItemText(hWnd, IDC_PLAIN, (char*) sztemp);
    }
    else
        MessageBox(hWnd,"Please fill all the necessary fields", "Upss..", MB_ICONERROR);
}

void ChangeBase(HWND hWnd, int ID)
{
    Big t;
    char szstring[2048];
    int edit_ids[] = {IDC_N, IDC_E, IDC_D, IDC_P, IDC_Q, IDC_CIPHER, IDC_HASH};
    int old_base,new_base,i;
    old_base = mip->IOBASE;
    switch(ID)
    {
    case IDC_B10:
        new_base = 10;
        break;
    case IDC_B16:
        new_base = 16;
        break;
    case IDC_B64:
        new_base = 64;
        break;
    }
    if (old_base != new_base )
    {

        for (i=0; i<8; i++)
        {
            if ( GetDlgItemText(hWnd, edit_ids[i], (char*) szstring, 2048))
            {
                t = szstring;
                mip->IOBASE = new_base;
                szstring << t;
                mip->IOBASE = old_base;
                SetDlgItemText(hWnd, edit_ids[i], (char*) szstring);
                RtlZeroMemory((char*) szstring, 2048);
            }
        }
    }
    mip->IOBASE = new_base;
    
}
BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
    struct md5_context md5_ctx;
    char szstring[2048];
    char md5_digest[16];
    Big t;

	switch (message)
	{    
	case WM_CLOSE:
		EndDialog(hWnd,0);
		break;
	case WM_COMMAND:
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
    		switch (LOWORD(wParam))
		    { 
		    case IDC_CMA:
                /* we need N,E,D for this attack*/
                SendMessage(GetDlgItem(hWnd,IDC_D), EM_SETREADONLY, FALSE, NULL);       
                SendMessage(GetDlgItem(hWnd,IDC_E), EM_SETREADONLY, FALSE, NULL);       
                
                /* set attack*/
                attack_type = IDC_CMA;
			    break;

            case IDC_WENER:
                /* we need only public key for this: N,E */
                DisableKey(hWnd);
                SendMessage(GetDlgItem(hWnd,IDC_E), EM_SETREADONLY, FALSE, NULL);       
                

                /* set attack*/
                attack_type = IDC_WENER;
                break;

            case IDC_WIENER:
                /* we need only public key for this:  N,E */
                DisableKey(hWnd);
                SendMessage(GetDlgItem(hWnd,IDC_E), EM_SETREADONLY, FALSE, NULL);       

                /* set attack*/
                attack_type = IDC_WIENER;                
                break;
            case IDC_LOW:
                /* we need only public key for this: N,E, but E=3*/
                DisableKey(hWnd);

                /* set attack*/
                attack_type = IDC_LOW;
                break;
            case IDC_B10:
                /* set base10*/
                ChangeBase(hWnd,IDC_B10);
                break;
            case IDC_B16:
                /* set base16*/
                ChangeBase(hWnd,IDC_B16);            
                break;
            case IDC_B64:
                /* set base64*/
                ChangeBase(hWnd,IDC_B64);
                break;
		    }
            break;
        case EN_CHANGE:
            if (LOWORD(wParam) == IDC_PLAIN)
            {
                /* compute md5 hash */
                GetDlgItemText(hWnd,IDC_PLAIN, (char*) szstring, 2048);
                md5_starts(&md5_ctx);
                md5_update(&md5_ctx, (unsigned char*) szstring, strlen((char*) szstring));
                md5_finish(&md5_ctx, (unsigned char*) md5_digest);
                t = from_binary(16, (char*) md5_digest);
                szstring << t;
                SetDlgItemText(hWnd,IDC_HASH, (char*) szstring);
            }

        }
		switch (LOWORD(wParam))
		{ 
		case IDC_COMPUTE:
            /* try to crack rsa key using selected attack */
            switch(attack_type)
            {
            case IDC_CMA:
                CMA_Attack(hWnd);
                break;
            case IDC_WENER:
                Wener_Attack(hWnd);
                break;
            case IDC_WIENER:
                Wiener_Attack(hWnd);
                break;
            case IDC_LOW:
                Low_Attack(hWnd);
                break;
            }
			break;
        case IDC_ABOUT:
            MessageBox(hWnd,"RSA ATTACKING TOOLKITT - bLaCk-eye [RET]\n\nJust a little tool that might help you when dealing\nvulnerable RSA keys.More of a toy :)\n\nHave fun!","About",MB_ICONINFORMATION);
            break;
        case IDC_LEAVE:
			EndDialog(hWnd,0);
			break;
        case IDC_ENCRYPT:
            RSA_Encrypt(hWnd);            
            break;
        case IDC_DECRYPT:
            RSA_Decrypt(hWnd);
            break;
		}
		break;
	case WM_INITDIALOG:
        SendMessage(hWnd,  WM_SETICON, ICON_SMALL,(LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
        SendMessage(hWnd,  WM_SETICON, ICON_BIG,(LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
        /* set default attack */
        CheckRadioButton(hWnd,IDC_CMA,IDC_LOW,IDC_CMA);

        /* set default base  */
        CheckRadioButton(hWnd,IDC_B16,IDC_B64,IDC_B16);

        mip=&precision;
        mip->IOBASE = 16;
        no_base = 16;
	break;
	}
     return 0;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	hInst=hInstance;
	InitCommonControls();
    DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DialogProc,0);
	return 0;
}
