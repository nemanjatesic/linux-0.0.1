#include <errno.h>
#include <fcntl.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const char * pathZaFile = ".encryptedList";
const char * pathZaFoldere = ".encrFolders";
int brojZaGlavniFile = 119;
int brojZaFoldere = 128;

int br = 0;
struct task_struct ** provera = &current;
extern int debugBr;

int hashString(char *string){
	int hash = 0;
	int deg = 37;
	int mod = 1050737;
	int i,duzina;
	duzina = strlen(string);
	for (i = 0 ; i < duzina ; i++){
		hash = (hash + string[i]*deg) % mod;
		deg = (deg*37) % mod;
	}
	return hash;
}

int isItInTheFile(struct m_inode * inode,int brojZaProveru){
	struct m_inode * inodeTmp;
	struct m_inode * inodePoredjenje;
	inodeTmp = iget(0x301,brojZaProveru);
	if (inodeTmp->i_size == 0){
		return -1;
	}
	struct buffer_head * bufferh;

	int inodeBlock;
	int block = 0;
	int brojac = 0;
	int brojZaFajl = 0;
	int brojZaKey = 0;
	int k = 0;
	while(block < inodeTmp->i_size){
		inodeBlock = bmap(inodeTmp,block/1024);
		bufferh = bread(inodeTmp->i_dev,inodeBlock);
		char stringTmp[20];
		brojac = 0;
		while (brojac < 1024){
			brojZaFajl = 0;
			brojZaKey = 0;
			k = 0;
			while(bufferh->b_data[brojac] != ',' && bufferh->b_data[brojac] != '\0'){
				stringTmp[k] = bufferh->b_data[brojac];
				brojac++;
				k++;
			}
			stringTmp[k] = '\0';
			// proverava da li je poslednja stvar dobra ili samo filler
			if (bufferh->b_data[brojac] == '\0' || bufferh->b_data[brojac] == ','){ 
				if (stringTmp[0] == ' ' || stringTmp[0] == '\0')
					break;
				// ako breakuje znaci da je bio filler ako ne breakuje onda je dobar kod i moze da se proveri u narednoj liniji
			}
			int br = 0;
			while (stringTmp[br] != ' '){
				brojZaFajl *= 10;
				brojZaFajl += stringTmp[br] - '0';
				br++;
			}
			br++;
			while (stringTmp[br] != '\0'){
				brojZaKey *= 10;
				brojZaKey += stringTmp[br] - '0';
				br++;
			}
			if (inode->i_num == brojZaFajl){
				return brojZaKey;
			}
			brojac++;
		}
		block += 1024;
		brelse(bufferh);
	}
	return -1;
}

int file_read(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	/*if (inode->i_num == brojZaGlavniFile)
		return -EPERM;*/
	//U slucaju da dodje problema i da sistem ne moze da se upali zbog kernel panika
	/*if (br == 0){
		struct m_inode * inode;
		inode = iget(0x301,brojZaGlavniFile);
		brojZaGlavniFile = inode->i_num;

		struct buffer_head * bh;
		int inodeBlock = bmap(inode,inode->i_size/1024);
		bh = bread(inode->i_dev,inodeBlock);
		bh->b_data[0] = '\0';
		bh->b_dirt = 1;
		inode->i_size = 0;
		brelse(bh);
		br++;
	}*/
	int tmp = isItInTheFile(inode,brojZaGlavniFile);
	if (tmp != -1){
		decryWithInode(inode,0,tmp);
	}
	// ----------------------------- //
	// Odavde pocinje prava funkcija //
	// ----------------------------- //
	int left,chars,nr;
	struct buffer_head * bh;

	if ((left=count)<=0)
		return 0;
	while (left) {
		if ((nr = bmap(inode,(filp->f_pos)/BLOCK_SIZE))) {
			if (!(bh=bread(inode->i_dev,nr)))
				break;
		} else
			bh = NULL;
		nr = filp->f_pos % BLOCK_SIZE;
		chars = MIN( BLOCK_SIZE-nr , left );
		filp->f_pos += chars;
		left -= chars;
		if (bh) {
			char * p = nr + bh->b_data;
			while (chars-->0)
				put_fs_byte(*(p++),buf++);
			brelse(bh);
		} else {
			while (chars-->0)
				put_fs_byte(0,buf++);
		}
	}
	inode->i_atime = CURRENT_TIME;
	// Moja provera //
	if (tmp != -1) {
		encryWithInode(inode,0,tmp);
		//iput(inode);
	}
	// Moja provera //
	return (count-left)?(count-left):-ERROR;
}

int file_write(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
	
	int tmp = isItInTheFile(inode,brojZaGlavniFile);
	if (tmp != -1){
		decryWithInode(inode,1,tmp);
	}

	// ----------------------------- //
	// Odavde pocinje prava funkcija //
	// ----------------------------- //
	off_t pos;
	int block,c;
	struct buffer_head * bh;
	char * p;
	int i=0;

/*
 * ok, append may not work when many processes are writing at the same time
 * but so what. That way leads to madness anyway.
 */
	if (filp->f_flags & O_APPEND)
		pos = inode->i_size;
	else
		pos = filp->f_pos;
	while (i<count) {
		if (!(block = create_block(inode,pos/BLOCK_SIZE)))
			break;
		if (!(bh=bread(inode->i_dev,block)))
			break;
		c = pos % BLOCK_SIZE;
		p = c + bh->b_data;
		bh->b_dirt = 1;
		c = BLOCK_SIZE-c;
		if (c > count-i) c = count-i;
		pos += c;
		if (pos > inode->i_size) {
			inode->i_size = pos;
			inode->i_dirt = 1;
		}
		i += c;
		while (c-->0)
			*(p++) = get_fs_byte(buf++);
		brelse(bh);
	}
	inode->i_mtime = CURRENT_TIME;
	if (!(filp->f_flags & O_APPEND)) {
		filp->f_pos = pos;
		inode->i_ctime = CURRENT_TIME;
	}
	// Moja provera //
	if (tmp != -1) {
		encryWithInode(inode,1,tmp);
		//iput(inode);
	}
	// Moja provera //
	return (i?i:-1);
}
