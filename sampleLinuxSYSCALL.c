#include<linux/linkage.h>
#include<linux/syscalls.h>
#include<linux/time.h>
#include<linux/rtc.h>
 
 
asmlinkage long sys_roylaracall(int pantherID)
{
        struct timeval Time;
        struct rtc_time tm;
        do_gettimeofday(&Time);
        rtc_time_to_tm((u32)(Time.tv_sec-sys_tz.tz_minuteswest*60), &tm);
       
        printk("sys_roy_lara called from process %d with panther ID %d.         \nCurrently %02d:%02d:%02d on %02d/%02d/%04d\n",                        current->pid, pantherID,
                //Time
                (((Time.tv_sec/3600)%24)+20)%12, //west to east
                (Time.tv_sec/60)%60, Time.tv_sec%60,
                //Date
                tm.tm_mon+1,tm.tm_mday, tm.tm_year+1900);
 
        return pantherID;
}
