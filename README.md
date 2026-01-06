# books

**THE PROBLEM**

i had a problem of wanting to start reading again but having too many unread books and not being able to pick one to start reading and finish the book.

**The Solution**
A command-line tool with a TUI to help you actually finish the books you start reading.

booktrack helps you:

- Track your unread books with page counts
- Randomly pick your next read
- Log daily reading progress
- Get accurate completion projections based on yout reading speed
- Stay motivated with stats

# View your library

./booktracker list

# Can't decide? Let fate decide

./booktracker pick

# Log your daily reading (the most important command!)

./booktracker log 25 # Read 25 pages today

# Check your progress

./booktracker status

# Output:

Current: 1984 by George Orwell

Progress: 25/328 pages (7.6%)

Reading speed: 25 pages/day

Estimated completion: January 14, 2025 (12 days)

Keep logging daily

./booktracker log 30
./booktracker log 22
./booktracker log 35

# Finished the book?

./booktracker finish

# View your reading stats

./booktracker stats

## Commands

**Library Management**

add <title> <pages> - Add a new unread book
list - Show all books (unread, current, finished)
delete <id> - Remove a book
edit-title <id> <title> - Update book title
edit-pages <id> <pages> - Update page count

**Reading Flow**

pick - Randomly select your next book
set-current <id> - Manually choose a book to read
log <pages> - Record pages read today
status - Show current book progress and projection
finish - Mark current book as complete

**Analytics**

stats - View reading statistics (speed, completion rate, etc.)

**Data Files**
All data stored in TSV format in the data/ directory:

books.tsv - Your book library
logs.tsv - Daily reading logs
notes.tsv - Book notes (future feature)
