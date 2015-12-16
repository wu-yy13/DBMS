create database library;
use library;
create table book(id int(10), name varchar(50), author varchar(50),d1 int(10) ,d2 int(10),primary key (id) );
INSERT INTO book VALUES 
 (200001,'Marias Diary (Plus S.)','Mark P. O. Morford',100082,5991),
 (200002,'Standing in the Shadows','Richard Bruce Wright',101787,2900),
 (200003,'Children of the Thunder','Carlo DEste',102928,3447),
 (200004,'The Great Gilly Hopkins','Gina Bari Kolata',101339,39),
 (200005,'Meine Juden--eure Juden','E. J. W. Barber',103089,206),
 (200006,'You Can Draw a Kangaroo','Amy Tan',101850,5296),
 (200007,'The Little Drummer Girl','Robert Cowley',104382,1006),
 (200008,'A Walk Through the Fire','Scott Turow',102008,8795),
 (200009,'The Nursing Home Murder','David Cordingly',102866,7380),
 (200010,'The Blanket of the Dark','Ann Beattie',103933,5242),
 (200011,'Not Without My Daughter','David Adams Richards',101177,567),
 (200012,'Introducing Halle Berry','Adam Lebor',104762,3505),
 (200013,'Men Who Love Too Little','Sheila Heti',103084,6131),
 (200014,'Once In a House On Fire','R. J. Kaiser',104024,4472),
 (200015,'Skindeep (Pan Horizons)','Jack Canfield',100670,4898),
 (200016,'A Voice Through a Cloud','Loren D. Estleman',101508,8322),
 (200017,'Master Georgie: A Novel','Robert Hendrickson',102615,1448),
 (200018,'Verdun (Lost Treasures)','Julia Oliver',102598,7459),
 (200019,'Der Pferdefl??sterer.','John Grisham',103834,6335),
 (200020,'Snowboarding to Nirvana','Toni Morrison',101085,8670),
 (200021,'Boys and Girls Together','The Onion',102228,3546),
 (200022,'Another Century of War?','Celia Brooks Brown',101834,2502),
 (200023,'A Book of Weather Clues','J. R. Parrish',100372,6066),
 (200024,'The Klutz Strikes Again','Mary-Kate &amp; Ashley Olsen',100733,8831),
 (200025,'Commitments (Arabesque)','Robynn Clairday',100289,2509),
 (200026,'The Third Twin: A Novel','Kathleen Duey',102335,4515),
 (200027,'The Dog Who Wouldnt Be','Rich Shapero',102789,6722),
 (200028,'Destined for the Throne','Michael Crichton',102197,6842),
 (200029,'Year of the Roasted Ear','C.S. Lewis',100428,8147);
select book.id from book where book.name = 'Year of the Roasted Ear';
select book.name from book where book.id = 200029;
 