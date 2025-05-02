import java.io.Serializable;
import java.util.Date;

public final class CalendarDate implements Serializable {
    private static final long serialVersionUID = 1L;
    private final Date date;
    private final String eventDescription;

    public CalendarDate(Date date, String eventDescription) {
        // Input validation
        if (date == null || eventDescription == null) {
            throw new IllegalArgumentException("Null values not allowed");
        }
        if (eventDescription.length() > 1000) {
            throw new IllegalArgumentException("Description exceeds 1000 chars");
        }
        
        // Defensive copies
        this.date = new Date(date.getTime());
        this.eventDescription = eventDescription;
    }

    // Immutable getters
    public Date getDate() {
        return new Date(date.getTime());
    }

    public String getEventDescription() {
        return eventDescription;
    }

    @Override
    public String toString() {
        return String.format("Date[%s]: %s", date, eventDescription);
    }
}
