#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <string.h>
#include <sys/stat.h>

extern short debugBr = 0;
extern int brojZaGlavniFile;
extern struct m_inode * isItInTheFile(struct m_inode * inode);
extern long startup_time;

char global_key[513] = "\0";

int sys_ftime()
{
	return -ENOSYS;
}

int sys_mknod()
{
	return -ENOSYS;
}

int sys_break()
{
	return -ENOSYS;
}

int sys_mount()
{
	return -ENOSYS;
}

int sys_umount()
{
	return -ENOSYS;
}

int sys_ustat(int dev,struct ustat * ubuf)
{
	return -1;
}

int sys_ptrace()
{
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_setgid(int gid)
{
	if (current->euid && current->uid)
		if (current->gid==gid || current->sgid==gid)
			current->egid=gid;
		else
			return -EPERM;
	else
		current->gid=current->egid=gid;
	return 0;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

int sys_time(long * tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc) {
		verify_area(tloc,4);
		put_fs_long(i,(unsigned long *)tloc);
	}
	return i;
}

int sys_setuid(int uid)
{
	if (current->euid && current->uid)
		if (uid==current->uid || current->suid==current->uid)
			current->euid=uid;
		else
			return -EPERM;
	else
		current->euid=current->uid=uid;
	return 0;
}

int sys_stime(long * tptr)
{
	if (current->euid && current->uid)
		return -1;
	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
	return 0;
}

int sys_times(struct tms * tbuf)
{
	if (!tbuf)
		return jiffies;
	verify_area(tbuf,sizeof *tbuf);
	put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
	put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
	put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
	put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
	return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&
	    end_data_seg < current->start_stack - 16384)
		current->brk = end_data_seg;
	return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
	int i;

	if (!pid)
		pid = current->pid;
	if (!pgid)
		pgid = pid;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->pid==pid) {
			if (task[i]->leader)
				return -EPERM;
			if (task[i]->session != current->session)
				return -EPERM;
			task[i]->pgrp = pgid;
			return 0;
		}
	return -ESRCH;
}

int sys_getpgrp(void)
{
	return current->pgrp;
}

int sys_setsid(void)
{
	if (current->uid && current->euid)
		return -EPERM;
	if (current->leader)
		return -EPERM;
	current->leader = 1;
	current->session = current->pgrp = current->pid;
	current->tty = -1;
	return current->pgrp;
}

int sys_oldolduname(void* v)
{
	printk("calling obsolete system call oldolduname\n");
	return -ENOSYS;
//	return (0);
}

int sys_uname(struct utsname * name)
{
	static struct utsname thisname = {
		"linux 0.01-3.x","nodename","release ","3.x","i386"
	};
	int i;

	if (!name) return -1;
	verify_area(name,sizeof *name);
	for(i=0;i<sizeof *name;i++)
		put_fs_byte(((char *) &thisname)[i],i+(char *) name);
	return (0);
}

int sys_umask(int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}

int sys_null(int nr)
{
	static int prev_nr=-2;
	if (nr==174 || nr==175) return -ENOSYS;

	if (prev_nr!=nr) 
	{
		prev_nr=nr;
//		printk("system call num %d not available\n",nr);
	}
	return -ENOSYS;
}

/* OS2019 */
extern long user_key_ptr, user_shift_ptr, user_alt_ptr;
extern int selected_layout, user_key_map_size;

void swap(char *a, char *b)                                                                                                                                                                       
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

void swapInt(int *a, int *b)                                                                                                                                                                       
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void enkriptuj(char * in, char * out) 
{
    char buffer[1025],keyTmp[513];
    char temp;
    strcpy(keyTmp, global_key);
    int col = strlen(global_key), row = 1024 / col, i, j, n, br = 0;
    for (i = 0; i < row; i++) {
        for (j = 0; j < col; j++)
            buffer[i * col + j] = in[br++];
    }
    for (i = 0; i < col; i++) {
        for (j = i + 1; j < col; j++) {
            if (keyTmp[i] > keyTmp[j]) {
                swap(keyTmp + i, keyTmp + j);
                for (n = 0; n < row; n++)
                    swap(buffer + n * col + i, buffer + n * col + j);
            }
        }
    }
    br = 0;
    for (j = 0; j < col; j++) {
        for (i = 0; i < row; i++)
            out[br++] = buffer[i * col + j];
    }
}

void dekriptuj(char * in, char * out)
{
	char buffer[1025], keyTmp[513];
	int keyIdx[513];
	int col = strlen(global_key), row = 1024 / col, i, j, br = 0;
	strcpy(keyTmp, global_key);
	for (i = 0; i < col; i++)
	    keyIdx[i] = i;
	for (j = 0; j < col; j++) {
	    for (i = 0; i < row; i++)
	        buffer[i * col + j] = in[br++];
	}
	for (i = 0; i < col; i++) {
	    for (j = i + 1; j < col; j++) {
	        if (keyTmp[i] > keyTmp[j]) {
	            swap(keyTmp + i, keyTmp + j);
	            swapInt(keyIdx + i, keyIdx + j);
	        }
	    }
	}
	for (i = 0; i < row; i++) {
	    for (j = 0; j < col; j++)
	        out[i * col + keyIdx[j]] = buffer[i * col + j];
	}
}

int sys_change_user_layout(const char *layout, int map)
{
	int i;
	char *maps[] = {(char*)user_key_ptr, (char*)user_shift_ptr, (char*)user_alt_ptr};
	if(map < 0 || map > 2)
	{
		return -1;
	}
	char *mp = maps[map], c;
	for(i = 0; i < user_key_map_size; ++i)
	{
		c = get_fs_byte(layout + i);
		*mp++ = c;
	}
	selected_layout = 2;
	return 0;
}

int sys_keyset(char *string)
{
	int i = 0;
	char str[513];
	while (get_fs_byte(string + i) != '\0'){
		str[i] = get_fs_byte(string + i);
		i++;
	}
	str[i] = '\0';
	if (i > 512) return -ENOTPOWOFTWO;
	while(i % 2 == 0){
		i = i / 2;
	}
	if (i != 1) return -ENOTPOWOFTWO;
	strcpy(global_key,str);
	return 0;
}

void reverse(char *s)
{
   	int length, c;
   	char *begin, *end, temp;
 
   	length = strlen(s);
   	begin = s;
   	end = s;
 
   	for (c = 0 ; c < length - 1 ; c++) end++;
 
   	for (c = 0 ; c < length / 2 ; c++){        
      	temp = *end;
      	*end = *begin;
     	*begin = temp;
      	begin++;
     	end--;
   	}
}

void brojToString(int broj, char * out)
{
	int tmp = 0;
	int i = 0;
	do{
		out[i++] = broj % 10 + '0';
	} while((broj /= 10) > 0);
	out[i] = '\0';
	reverse(out);
}

void shiftStringToLeft(int from, int howMuch, char * out)
{
	int i,j;
	int size = strlen(out);
	for (j = 0 ; j < howMuch ; j++){
		for (i = from ; i < size ; i++){
			out[i] = out[i+1];
		}
	}
}

int doesItContainString(char * big, char * small)
{
	int bigSize = strlen(big);
	int smallSize = strlen(small);
	if (bigSize < smallSize)
		return -1;
	int i,j,boolean;

	for (i = 0 ; i < bigSize - smallSize + 1 ; i++){
		boolean = 1;
		for (j = 0 ; j < smallSize ; j++){
			if (small[j] != big[i + j]){
				boolean = 0;
				break;
			}
		}
		if (boolean) return i;
	}
	return -1;
}

void izbrisiIzFajla(struct m_inode * inodeZaBrisanje)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	inode = iget(0x301,brojZaGlavniFile);
	int block = inode->i_size;
	int inodeBlock;

	inodeBlock = bmap(inode,block/1024);
	bh = bread(inode->i_dev,inodeBlock);

	char buffer[5];
	brojToString(inodeZaBrisanje->i_num,buffer);

	int brojGdeKrece = doesItContainString(bh->b_data,buffer);
	
	// +1 zbog toga sto zelimo da obrisemo i ' ' (razmak koji se nalazi u fajlu)
	// ali ako je prvi u fajlu onda zelim samo orignalni broj bez +1
	int brojPomeranja = strlen(buffer) + 1; 
	if (brojGdeKrece > 0) brojGdeKrece--;
	if (strlen(buffer) == strlen(bh->b_data)/*inode->i_size*/) brojPomeranja--;
	// Ovaj else sluzi zato sto zelimo da krene od ' ' ne odakle ga je nasao
	
	// Nije moguc slucaj ali nisam siguran
	if (brojGdeKrece == -1){ 
		brelse(bh);
		iput(inode);
		return;
	}
	
	shiftStringToLeft(brojGdeKrece, brojPomeranja, bh->b_data);
	inode->i_size -= brojPomeranja;
	inode->i_ctime = CURRENT_TIME;
	inode->i_dirt = 1;
	bh->b_dirt = 1;
	if (debugBr) printk("Ovoliko je string : |%s|,ovoliko je duzina stringa : %d, inode size : %d\n", bh->b_data,strlen(bh->b_data),inode->i_size);
	//printk("Ovoliko je string : %s,inode size : %d\n", bh->b_data,inode->i_size);
	brelse(bh);
	iput(inode);
}

void upisiUFajl(struct m_inode * inodeZaUpis)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	inode = iget(0x301,brojZaGlavniFile);
	int block = inode->i_size;
	int inodeBlock,i;

	inodeBlock = bmap(inode,block/1024);
	bh = bread(inode->i_dev,inodeBlock);
	if (debugBr) printk("Ovoliko je string : |%s|,ovoliko je duzina stringa : %d, inode size : %d\n", bh->b_data,strlen(bh->b_data),inode->i_size);

	int brojKraja = 0;
	for (i = 0 ; i < 1024 ; i++) {
		if (bh->b_data[i] == '\0'){
			brojKraja = i;
			break;
		}
	}

	char buffer[5];
	brojToString(inodeZaUpis->i_num,buffer);

	if (brojKraja + strlen(buffer) + 1 > 1024){
		brelse(bh);
		iput(inode);
		return;
	}
	//printk("Ovo je :%s, ovo je broj : %d, ovoliko ce biti i : %d, ici ce do ovde : %d\n",bh->b_data,brojKraja,brojKraja + 1,brojKraja + strlen(buffer) + 1);
	if (strlen(bh->b_data) != 0/* || inode->i_size != 0*/) bh->b_data[brojKraja] = ' ';
	else brojKraja--;
	int j = 0;

	for (i = brojKraja + 1 ; i < brojKraja + strlen(buffer) + 1 ; i++){
		bh->b_data[i] = buffer[j++];
	}

	bh->b_data[i] = '\0';
	bh->b_dirt = 1;
	//printk("This shit 1: |%s|, this big : %d",bh->b_data,i);
	inode->i_ctime = CURRENT_TIME;
	inode->i_size = i;
	inode->i_dirt = 1;
	if (debugBr) printk("Ovoliko je string : |%s|,ovoliko je duzina stringa : %d, inode size : %d\n", bh->b_data,strlen(bh->b_data),inode->i_size);
	brelse(bh);
	iput(inode);
}

int sys_encry(char *string)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	char stringData[1024];
	int i = 0;

	inode = namei(string);

	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
	if (inode == NULL)
		return -ENOFILE;
	if (S_ISDIR(inode->i_mode))
		return -EISDIR;
	if (global_key[0] == '\0')
		return -EKEYNOTFOUND;

	struct m_inode * tmpNode;
	tmpNode = isItInTheFile(inode);
	if (tmpNode == NULL){
		upisiUFajl(inode);
	}else{
		iput(inode);
		return -EAENCR;
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		enkriptuj(bh->b_data,stringData);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		bh->b_dirt = 1;
		brelse(bh);
	}
	iput(inode);
	return 0;
}

int sys_decry(char *string)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	char stringData[1024];
	int i = 0;

	inode = namei(string);

	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
	if (inode == NULL)
		return -ENOFILE;
	if (S_ISDIR(inode->i_mode))
		return -EISDIR;
	if (global_key[0] == '\0')
		return -EKEYNOTFOUND;

	struct m_inode * tmpNode;
	tmpNode = isItInTheFile(inode);
	if (tmpNode == NULL){
		iput(inode);
		return -EAENCR;
	}else{
		izbrisiIzFajla(inode);
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		dekriptuj(bh->b_data,stringData);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		bh->b_dirt = 1;
		brelse(bh);
	}
	iput(inode);
	return 0;
}

int encryWithInode(struct m_inode * inodeTmp, int bool)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	char stringData[1024];
	int i = 0;
	inode = inodeTmp;

	if (inode == NULL){
		return -ENOFILE;
	}
	if (S_ISDIR(inode->i_mode)){
		return -EISDIR;
	}
	if (global_key[0] == '\0'){
		return -EKEYNOTFOUND;
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		enkriptuj(bh->b_data,stringData);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		if (bool) bh->b_dirt = 1;
		brelse(bh);
	}
	return 0;
}

int decryWithInode(struct m_inode * inodeTmp, int bool)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	char stringData[1024];
	int i = 0;
	inode = inodeTmp;

	if (inode == NULL){
		return -ENOFILE;
	}
	if (S_ISDIR(inode->i_mode)){
		return -EISDIR;
	}
	if (global_key[0] == '\0'){
		return -EKEYNOTFOUND;
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		dekriptuj(bh->b_data,stringData);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		if (bool) bh->b_dirt = 1;
		brelse(bh);
	}
	return 0;
}

int sys_keyclear()
{
	global_key[0] = '\0';
	return 0;
}

int sys_zapocni(char * string, int mode)
{
	if (mode == 2){
		debugBr = !debugBr;
		return 0;
	}

	struct m_inode * inode;
	inode = namei(string);
	brojZaGlavniFile = inode->i_num;

	if (mode == 1){
		struct buffer_head * bh;
		int inodeBlock = bmap(inode,inode->i_size/1024);
		bh = bread(inode->i_dev,inodeBlock);
		bh->b_data[0] = '\0';
		bh->b_dirt = 1;
		inode->i_size = 0;
		brelse(bh);
	}

	iput(inode);
	return 0;
}