#include <errno.h>
#include <fcntl.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <asm/segment.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const char * pathZaFile = ".encryptedList";
int brojZaGlavniFile = 137;
int br = 0;

struct m_inode * isItInTheFile(struct m_inode * inode){
	struct m_inode * inodeTmp;
	struct m_inode * inodePoredjenje;
	inodeTmp = iget(0x301,brojZaGlavniFile);
	if (inodeTmp->i_size == 0){
		return NULL;
	}
	int inodeTmpBlock = bmap(inodeTmp,inodeTmp->i_size/1024);
	int poreditiSaOvime = bmap(inode,inode->i_size/1024);

	struct buffer_head * bufferh;

	int block = inodeTmp->i_size;
	int inodeBlock;

	int brojac = 0;
	inodeBlock = bmap(inodeTmp,block/1024);
	bufferh = bread(inodeTmp->i_dev,inodeBlock);
	char * strTmp;
	char * tmp;
	int brojZaFajl = 0;
	for (brojac = 0; brojac < 1024; brojac++){
		if (bufferh->b_data[brojac] == ' ' || bufferh->b_data[brojac] == '\0'){
			if (!brojZaFajl)
				continue;
			inodePoredjenje = iget(0x301,brojZaFajl);
			if (bmap(inodePoredjenje,inodePoredjenje->i_size/1024) == 0){
				// Doslo je do greske pri ucitavanju (fajl ne postoji ili je losa putanja)
				brojZaFajl = 0;
				continue;
			}
			if (bmap(inodePoredjenje,inodePoredjenje->i_size/1024) == poreditiSaOvime){
				//printk("Usao ovde\n");
				return inodePoredjenje;
			}	
			if (bufferh->b_data[brojac] == '\0'){
				iput(inodePoredjenje);
				break;
			}
			brojZaFajl = 0;
			iput(inodePoredjenje);
			continue;
		}
		brojZaFajl *= 10;
		brojZaFajl += bufferh->b_data[brojac] - '0';
	}
	return NULL;
}

int file_read(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
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

	struct m_inode * tmp;
	tmp = isItInTheFile(inode);
	if (tmp != NULL){
		decryWithInode(tmp,0);
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
	if (tmp != NULL) {
		encryWithInode(tmp,0);
		iput(tmp);
	}
	// Moja provera //
	return (count-left)?(count-left):-ERROR;
}

int file_write(struct m_inode * inode, struct file * filp, char * buf, int count)
{
	if (inode->i_num == brojZaGlavniFile)
		return -EPERM;
	
	struct m_inode * tmp;
	
	tmp = isItInTheFile(inode);
	if (tmp != NULL){
		decryWithInode(tmp,1);
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
	if (tmp != NULL) {
		encryWithInode(tmp,1);
		iput(tmp);
	}
	// Moja provera //
	return (i?i:-1);
}
