# booktrack v2.0

**THE PROBLEM**

I had a problem of wanting to start reading again but having too many unread books and not being able to pick one to start reading and finish the book.

**THE SOLUTION**

A command-line tool with a full-screen TUI to help you actually finish the books you start reading.

booktrack helps you:

- Track your unread books with page counts
- Randomly pick your next read
- Log daily reading progress
- Get accurate completion projections based on your reading speed
- Stay motivated with stats

## Quick Start

```bash
# Build
make clean && make

# Launch TUI (recommended)
./booktracker tui

# Or use CLI commands
./booktracker add "1984" 328
./booktracker list
./booktracker pick
./booktracker log 157
```

## TUI Features

The interactive TUI provides:

- **Books** - Browse, edit, delete, and start reading (arrow keys + e/d/enter)
- **Log pages** - Track your reading progress (enter page you're on)
- **Pick book** - Let fate decide your next read
- **Add book(s)** - Add new books or start reading existing ones
- **Finish book** - Mark current book complete
- **Projection** - See how many books you can finish by year-end
- **Stats** - View reading speed and completion metrics

## CLI Commands

### Library Management

- `add <title> <pages>` - Add a new unread book
- `list` - Show all books (unread, reading, finished)
- `delete <id>` - Remove a book
- `edit-title <id> <title>` - Update book title
- `edit-pages <id> <pages>` - Update page count

### Reading Flow

- `pick` - Randomly select your next book
- `set-current <id>` - Manually choose a book to read
- `log <page>` - Record what page you're on now
- `finish` - Mark current book as complete

### Analytics

- `projection` - Year-end reading projection
- `stats` - View reading statistics

## Data Files

All data stored in TSV format in the `data/` directory:

- `books.tsv` - Your book library
- `logs.tsv` - Daily reading logs
