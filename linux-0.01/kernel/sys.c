#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <string.h>
#include <sys/stat.h>
#define BROJ 8

extern short debugBr = 0;
extern int brojZaGlavniFile;
extern struct m_inode * isItInTheFile(struct m_inode * inode);
extern long startup_time;
extern int hashString(char *string);

char nizProcesa[BROJ][513];
short nizProcesIDova[NR_TASKS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
long nizProcesVremena[BROJ] = {-1,-1,-1,-1,-1,-1,-1,-1};
char nizSlobodnih[BROJ] = {1,1,1,1,1,1,1,1};
char global_key[513] = "\0";
long brojacGlobal = -1;
long prviSledeciBrojac = -1;
struct task_struct **tmp;

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
		printk("system call num %d not available\n",nr);
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

void enkriptuj(char * in, char * out, char * stringKey) 
{
    char buffer[1025],keyTmp[513];
    char temp;
    strcpy(keyTmp, stringKey);
    int col = strlen(stringKey), row = 1024 / col, i, j, n, br = 0;
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

void dekriptuj(char * in, char * out, char * stringKey)
{
	char buffer[1025], keyTmp[513];
	int keyIdx[513];
	int col = strlen(stringKey), row = 1024 / col, i, j, br = 0;
	strcpy(keyTmp, stringKey);
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

int getIndexCurrenta(){
	int i;
	for (i = 0 ; i < NR_TASKS ; i++){
		if (task[i] == current){
			return i;
		}
	}
	return 0;
}

int sys_keyset(int mode,char *string)
{
	if (mode != 0 && mode != 1)
		return -ERANGE;
	int i = 0;
	char str[513];
	while (get_fs_byte(string + i) != '\0'){
		str[i] = get_fs_byte(string + i);
		i++;
	}
	str[i] = '\0';

	if (i > 512) return -ENOTPOWOFTWO;
	if(!((i != 0) && ((i &(i - 1)) == 0))) 
		return -ENOTPOWOFTWO;

	int vreme_tmp = jiffies/HZ;

	if (!mode){
		strcpy(global_key,str);
		brojacGlobal = vreme_tmp + 120;
		return 0;
	}

	if (mode){
		char boolean = 1;
		int index = getIndexCurrenta();

		if (nizProcesIDova[index] != -1){
			strcpy(nizProcesa[nizProcesIDova[index]],str);
			nizProcesVremena[nizProcesIDova[index]] = vreme_tmp + 45;
			boolean = 0;
		}

		if (boolean){
			short brojMesta = -1;
			int j;
			for (j = 0 ; j < BROJ ; j++){
				if (nizSlobodnih[j] == 1){
					brojMesta = j;
				}
			}
			if (brojMesta == -1){
				panic("Too many processes in same time.");
			}
			nizProcesIDova[index] = brojMesta;
			nizProcesVremena[nizProcesIDova[index]] = vreme_tmp + 45;
			nizSlobodnih[brojMesta] = 0;
			strcpy(nizProcesa[brojMesta],str);
			if (prviSledeciBrojac == -1 || prviSledeciBrojac > vreme_tmp + 45){
				prviSledeciBrojac = vreme_tmp + 45;
			}
		}
	}

	return 0;
}


int funkcija(void)
{
	long timeTMP = jiffies/HZ;
	if (brojacGlobal <= timeTMP && brojacGlobal != -1){
		global_key[0] = '\0';
		brojacGlobal = -1;
	}
	if (prviSledeciBrojac > timeTMP || prviSledeciBrojac == -1)
		return 0;
	int i;
	prviSledeciBrojac = -1;
	for (i = 0 ; i < NR_TASKS ; i++){
		if (nizProcesIDova[i] != -1){
			if(nizProcesVremena[nizProcesIDova[i]] <= timeTMP){
				nizProcesVremena[nizProcesIDova[i]] = -1;
				nizSlobodnih[nizProcesIDova[i]] = 1;
				nizProcesa[nizProcesIDova[i]][0] = '\0';
				nizProcesIDova[i] = -1;
				continue;
			}
		}else {
			continue;
		}
		if (nizProcesVremena[nizProcesIDova[i]] != -1 && (nizProcesVremena[nizProcesIDova[i]] < prviSledeciBrojac || prviSledeciBrojac == -1)){
			prviSledeciBrojac = nizProcesVremena[nizProcesIDova[i]];
		}
	}
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

void fillStuff(struct buffer_head * bh, struct m_inode * inode){
	int i;
	for (i = strlen(bh->b_data) ; i <= 1023 ; i++){
		bh->b_data[i] = '\0';
		inode->i_size += 1;
	}
}

void izbrisiIzFajla(struct m_inode * inodeZaBrisanje, int duzinaHesha)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	inode = iget(0x301,brojZaGlavniFile);

	char buffer[5];
	brojToString(inodeZaBrisanje->i_num,buffer);

	int block = 0;
	int inodeBlock;
	int brojGdeKrece;
	int brojPomeranja = strlen(buffer) + 2 + duzinaHesha;

	while(block < inode->i_size){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);
		brojGdeKrece = doesItContainString(bh->b_data,buffer);
		if (brojGdeKrece != -1){
			shiftStringToLeft(brojGdeKrece, brojPomeranja, bh->b_data);
			if (block + 1024 > inode->i_size)
				inode->i_size -= brojPomeranja;
			inode->i_dirt = 1;
			brelse(bh);
			break;
		}
		block += 1024;
		brelse(bh);
	}

	// +1 zbog toga sto zelimo da obrisemo i ' ' (razmak koji se nalazi u fajlu)
	// ali ako je prvi u fajlu onda zelim samo orignalni broj bez +1
	/*int brojPomeranja = strlen(buffer) + 1; 
	if (brojGdeKrece > 0) brojGdeKrece--;
	if (strlen(buffer) == strlen(bh->b_data)) brojPomeranja--;
	// Ovaj else sluzi zato sto zelimo da krene od ' ' ne odakle ga je nasao
	
	shiftStringToLeft(brojGdeKrece, brojPomeranja, bh->b_data);
	inode->i_size -= brojPomeranja;
	inode->i_ctime = CURRENT_TIME;
	inode->i_dirt = 1;
	bh->b_dirt = 1;
	if (debugBr) printk("Ovoliko je string : |%s|,ovoliko je duzina stringa : %d, inode size : %d\n", bh->b_data,strlen(bh->b_data),inode->i_size);
	//printk("Ovoliko je string : %s,inode size : %d\n", bh->b_data,inode->i_size);
	brelse(bh);*/
	//iput(inode);
}

void upisiUFajl(struct m_inode * inodeZaUpis)
{
	int brojDuzine = 0;
	int numTmp = inodeZaUpis->i_num;
	while(numTmp > 0){
		numTmp /= 10;
		brojDuzine++;
	}
	brojDuzine = brojDuzine + 2; // zbog ' ' i zbog ','

	int hashBroj;
	int indexCurr = getIndexCurrenta();
	if (nizProcesIDova[indexCurr] != -1){
		hashBroj = hashString(nizProcesa[nizProcesIDova[indexCurr]]);
	}else {
		hashBroj = hashString(global_key);
		if (debugBr) printk("ovde sam i broj je ovoliko : %d, |%s|\n",hashBroj,global_key);
	}

	numTmp = hashBroj;
	while(numTmp > 0){
		numTmp /= 10;
		brojDuzine++;
	}

	char buffer[5];
	brojToString(inodeZaUpis->i_num,buffer);

	char bufferHash[11];
	brojToString(hashBroj,bufferHash);

	char ceoString[20] = "";
	strcat(ceoString,buffer);
	strcat(ceoString," ");
	strcat(ceoString,bufferHash);
	strcat(ceoString,",");

	struct m_inode * inode;
	struct buffer_head * bh;
	inode = iget(0x301,brojZaGlavniFile);

	int block = 0;
	int inodeBlock;
	char boolean = 1;

	if (debugBr) printk("inode_i_size : %d, string : %s\n",inode->i_size,ceoString);

	while(block < inode->i_size){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);
		if (strlen(bh->b_data) + brojDuzine < 1024){
			boolean = 0;
			strcat(bh->b_data,ceoString);
			inode->i_ctime = CURRENT_TIME;
			inode->i_size += strlen(ceoString);
			inode->i_dirt = 1; 
			if (debugBr) printk("usao ovde : |%s|%d|%d|\n","",strlen(bh->b_data),inode->i_size);
			bh->b_dirt = 1;
			brelse(bh);
			break;
		}

		block = block + 1024;
		bh->b_dirt = 1;
		brelse(bh);
	}

	if (boolean){
		if (inode->i_size % 1024 != 0)
			fillStuff(bh,inode);
		inodeBlock = create_block(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);
		int brojBHKraj = strlen(bh->b_data);
		strcat(bh->b_data,ceoString);
		inode->i_ctime = CURRENT_TIME;
		inode->i_size += strlen(ceoString);
		inode->i_dirt = 1; 
		bh->b_dirt = 1;
		if (debugBr) printk("CAO usao ovde : |%s|%d|%d|\n",bh->b_data,strlen(bh->b_data),inode->i_size);
		brelse(bh);
	}
}

int sys_encry(char *string)
{
	struct m_inode * inode;
	struct buffer_head * bh;
	char stringData[1024];
	int i = 0;

	inode = namei(string);
	
	/// DEBUG SKLONITI POSLEEEEEEEEEEEEEE
	/*upisiUFajl(inode);

	return 0;*/
	/// DEBUG SKLONITI POSLEEEEEEEEEEEEEE
	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
	if (inode == NULL)
		return -ENOFILE;
	if (S_ISDIR(inode->i_mode))
		return -EISDIR;
	int curr = getIndexCurrenta();
	if (global_key[0] == '\0' && nizProcesIDova[curr] == -1)
		return -EKEYNOTFOUND;

	char keyUsed[514];
	strcpy(keyUsed,global_key);
	int tmpNode = isItInTheFile(inode);
	if (nizProcesIDova[curr] != -1){
		strcpy(keyUsed,nizProcesa[nizProcesIDova[curr]]);
	}

	if (tmpNode == -1){
		upisiUFajl(inode);
	}else{
		//iput(inode);
		return -EAENCR;
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		enkriptuj(bh->b_data,stringData,keyUsed);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		bh->b_dirt = 1;
		brelse(bh);
	}
	//iput(inode);
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
	int curr = getIndexCurrenta();
	if (global_key[0] == '\0' && nizProcesIDova[curr] == -1)
		return -EKEYNOTFOUND;

	char keyUsed[514];
	strcpy(keyUsed,global_key);

	int tmpNode = isItInTheFile(inode);
	if (tmpNode == -1){
		//iput(inode);
		return -EAENCR;
	}else{
		char tmpStr[10];
		brojToString(tmpNode,tmpStr);
		int duzina = strlen(tmpStr);
		// ako postoji key za proces 
		if (nizProcesIDova[curr] != -1){
			if (hashString(nizProcesa[nizProcesIDova[curr]]) == tmpNode){
				izbrisiIzFajla(inode,duzina);
				strcpy(keyUsed,nizProcesa[nizProcesIDova[curr]]);
			}else{ // ako hesh nije dobar probamo sa globalnim ako postoji
				if (global_key[0] != '\0'){
					if (hashString(global_key) == tmpNode){
						izbrisiIzFajla(inode,duzina);
					}else {
						return -EWRONGKEY;
					}
				}else {
					return -EWRONGKEY;
				}
			}
		}else { // ako ne postoji key za proces znamo da postoji za globalni onda
			if (hashString(global_key) == tmpNode){
				izbrisiIzFajla(inode,duzina);
			}else {
				return -EWRONGKEY; // ako globalni key hash nije isti kao hash za koji smo zapamtili return error
			}
		}
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		dekriptuj(bh->b_data,stringData,keyUsed);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		bh->b_dirt = 1;
		brelse(bh);
	}
	//iput(inode);
	return 0;
}

int encryWithInode(struct m_inode * inodeTmp, int bool, int hashedKey)
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
	char keyUsed[514];
	strcpy(keyUsed,global_key);
	int curr = getIndexCurrenta();
	char boolean = 0;
	if (nizProcesIDova[curr] != -1){
		strcpy(keyUsed,nizProcesa[nizProcesIDova[curr]]);
		boolean = 1;
	}

	if (global_key[0] == '\0'){
		if (!boolean)
			return -EKEYNOTFOUND;
	}

	int block = inode->i_size;
	int inodeBlock;

	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		enkriptuj(bh->b_data,stringData,keyUsed);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		if (bool) bh->b_dirt = 1;
		brelse(bh);
	}
	return 0;
}

int decryWithInode(struct m_inode * inodeTmp, int bool, int hashedKey)
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
	char keyUsed[514];
	keyUsed[0] = '\0';
	int curr = getIndexCurrenta();
	char boolean = 0;
	if (nizProcesIDova[curr] != -1){
		if (hashString(nizProcesa[nizProcesIDova[curr]]) == hashedKey){
			strcpy(keyUsed,nizProcesa[nizProcesIDova[curr]]);
			boolean = 1;
		}
	}
	if (hashString(global_key) == hashedKey){
		strcpy(keyUsed,global_key);
	}

	if (keyUsed[0] == '\0'){
		return -EKEYNOTFOUND;
	}

	int block = inode->i_size;
	int inodeBlock;
	while(block > 0){
		inodeBlock = bmap(inode,block/1024);
		bh = bread(inode->i_dev,inodeBlock);

		dekriptuj(bh->b_data,stringData,keyUsed);

		for (i = 0 ; i < 1024 ; i++){
			bh->b_data[i] = stringData[i];
		}

		block = block - 1024;
		if (bool) bh->b_dirt = 1;
		brelse(bh);
	}
	return 0;
}

int sys_keyclear(int mode)
{
	if (mode != 0 && mode != 1)
		return -ERANGE;
	if (!mode){
		brojacGlobal = -1;
		global_key[0] = '\0';
		return 0;
	}
	if (mode){
		int index = getIndexCurrenta();
		if (nizProcesIDova[index] == -1){
			return 0;
		}
		nizSlobodnih[nizProcesIDova[index]] = 1;
		nizProcesa[nizProcesIDova[index]][0] = '\0';
		nizProcesVremena[nizProcesIDova[index]] = -1;
		nizProcesIDova[index] = -1;
		return 0;
	}
}

int sys_zapocni(char * string, int mode)
{
	if (mode == 2){
		tmp = &current;
		debugBr = !debugBr;
		return 0;
	}

	struct m_inode * inode;
	inode = namei(string);
	brojZaGlavniFile = inode->i_num;
	if(debugBr) printk("ovoliko : %d",brojZaGlavniFile);

	if (mode == 1){
		struct buffer_head * bh;
		int inodeBlock = bmap(inode,inode->i_size/1024);
		bh = bread(inode->i_dev,inodeBlock);
		bh->b_data[0] = '\0';
		bh->b_dirt = 1;
		inode->i_size = 0;
		brelse(bh);
	}

	//iput(inode);
	return 0;
}

int sys_menjanjeEchoa(char boolean){
	if (!boolean)
		tty_table[0].termios.c_lflag = tty_table[0].termios.c_lflag & ~0000010;
	else 
		tty_table[0].termios.c_lflag = tty_table[0].termios.c_lflag | 0000010;
}