@ stdcall -stub YAbortPrinter(ptr long)
@ stdcall -stub YAddJob(long long ptr long ptr long)
@ stdcall -stub YDriverUnloadComplete(wstr long)
@ stdcall -stub YEndDocPrinter(long long)
@ stdcall -stub YEndPagePrinter(long long)
@ stdcall -stub YFlushPrinter(ptr ptr long ptr long long)
@ stdcall -stub YGetPrinter(ptr long ptr long ptr long)
@ stdcall YGetPrinterDriver2(ptr wstr long ptr long ptr long long ptr ptr long)
@ stdcall -stub YGetPrinterDriverDirectory(wstr wstr long ptr long ptr long)
@ stdcall -stub YReadPrinter(ptr ptr long ptr long)
@ stdcall -stub YSeekPrinter(ptr int64 ptr long long long)
@ stdcall -stub YSetJob(ptr long long ptr long long)
@ stdcall -stub YSetPort(wstr wstr long ptr long)
@ stdcall -stub YSetPrinter(ptr long ptr long long)
@ stdcall -stub YSplReadPrinter(ptr ptr long long)
@ stdcall -stub YStartDocPrinter(ptr long ptr long)
@ stdcall -stub YStartPagePrinter(ptr long)
@ stdcall -stub YWritePrinter(ptr ptr long ptr long)
