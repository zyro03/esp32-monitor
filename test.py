\begin{table}[htbp]
    \centering
    \small
    \caption{Wyniki pięciu prób alarmowych.}
    \label{tab:testy_alarmow}
    \setlength{\tabcolsep}{3pt}
    \renewcommand{\arraystretch}{1.2}
    \begin{tabular}{
        @{}c c c c c c
        p{0.21\textwidth}
        p{0.20\textwidth}@{}
    }
        \toprule
        Próba &
        \shortstack{Pomiar\\[$^\circ$C / \%]} &
        \shortstack{$T_{\min}$\\[$^\circ$C]} &
        \shortstack{$T_{\max}$\\[$^\circ$C]} &
        \shortstack{$H_{\min}$\\[\%]} &
        \shortstack{$H_{\max}$\\[\%]} &
        Komunikat LCD &
        Informacja w panelu \\
        \midrule

        1 & 24,0 / 46,7 & 25 & 40 & 10 & 90 &
        \texttt{TEMP ZA NISKA} &
        Temperatura poniżej minimum \\

        2 & 24,5 / 86,7 & 10 & 24 & 10 & 90 &
        \texttt{TEMP ZA WYSOKA} &
        Temperatura powyżej maksimum \\

        3 & 24,3 / 62,6 & 10 & 40 & 65 & 90 &
        \texttt{WILG ZA NISKA} &
        Wilgotność poniżej minimum \\

        4 & 25,1 / 65,8 & 10 & 40 & 10 & 65 &
        \texttt{WILG ZA WYSOKA} &
        Wilgotność powyżej maksimum \\

        5 & 24,4 / 64,1 & 25 & 40 & 65 & 90 &
        \texttt{T:NIS H:NIS} &
        Niska temperatura i wilgotność \\

        \bottomrule
    \end{tabular}
\end{table}